#include<stdio.h>
#include<math.h>
#include "sebal_eta.h"
#define PI 3.1415927

double idx(double min, double max, double pixel)
{
	return (max-pixel)/(max-min);
}

// NDVI
double ndvi(double red, double nir){
	double result;
	if(red+nir<=0.001){
		result=-1.0;
	}else{
		result=(nir-red)/(red+nir);
	}
	return result;
}

// Broadband albedo MODIS
double bb_alb_modis( double redchan, double nirchan, double chan3, double chan4, double chan5, double chan6, double chan7 )
{
	double	result;
	result =(0.22831*redchan+ 0.15982*nirchan+ 0.09132*(chan3+chan4+chan5)+ 0.10959*chan6+ 0.22831*chan7);
	return result;
}

// Wim delta T (soil-air) equation
double deltat_wim(double tempk){
	double result;
	result = 0.3225 * tempk - 91.743;
	if(result<1){
		result=1.0;
	} else if(result>13){
		result=13.0;
	}
	return result;
}

// Average Solar Diurnal Radiation after Bastiaanssen (1995)
double solar_day(double lat, double doy, double tsw )
{
	double ws, cosun, latrad, deltarad, ds, result;

	//Sun-Earth Distance (ds; A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
	//Solar declination (delta; radians)
	deltarad = 0.4093*sin((2*PI*doy/365)-1.39);
	//Convert latitude in radians
	latrad =  lat * PI / 180.0;

	//Convert latitude in radians
	ws = acos(-tan(latrad)*tan(deltarad));
	cosun = 2*(ws*sin(deltarad)*sin(latrad)+cos(deltarad)*cos(latrad)*sin(ws));

	result = ( cosun * 1367 * tsw ) / ( 2 * ws * ds * ds );

	return result;

}

// Average Diurnal Net Radiation after Bastiaanssen (1995)
double r_net_day( double bbalb, double solar, double tsw )
{
	return (((1.0 - bbalb)*solar)-(110.0*tsw));
}

// Average Diurnal Potential ET after Bastiaanssen (1995)
double et_pot_day( double rnetd, double tempk, double roh_w )
{
	double latent, result;

	latent=(2.501-(0.02361*(tempk-273.15)))*1000000.0;
	result = (rnetd*86400*1000.0)/(latent*roh_w);

	return result;
}

// Instantaneous net radiation (W/m2)
double r_net( double bbalb, double ndvi, double tempk, double dtair,  double e0, double tsw, double doy, double utc, double sunzangle )
{
	/* Tsw =  atmospheric transmissivity single-way (~0.7 -) */
	/* DOY = Day of Year */
	/* utc = UTC time of sat overpass*/
	/* sunzangle = sun zenith angle at sat. overpass */
	/* tair = air temperature (approximative, or met.station) */

	double 	Kin=0.0, Lin=0.0, Lout=0.0, Lcorr=0.0, result=0.0;
	double 	ds=0.0, e_atm=0.0, delta=0.0;
	double	tsw_for_e_atm=0.7; //Special tsw, consider it a coefficient

// 	printf("**rnet: bbalb = %5.3f\n\tndvi = %5.3f\n\ttempk = %5.3f\n\te0 = %5.3f\n\ttsw = %5.3f\n\tdoy = %f\n\tutc = %5.3f\n\tsunzangle = %5.3f\n\tdtair = %5.3f\n",bbalb,ndvi,tempk,e0,tsw,doy,utc,sunzangle,dtair);

	// Atmospheric emissivity (Bastiaanssen, 1995)
	e_atm	=  1.08 * pow(-log(tsw_for_e_atm),0.265) ;
	// Atmospheric emissivity (Pawan, 2004)
// 	e_atm	= 0.85 * pow(-log(tsw),0.09);
// 	printf("rnet: e_atm = %5.3f\n",e_atm);

	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365);
// 	printf("rnet: ds = %lf\n",ds);
	delta = 0.4093*sin((2*PI*doy/365)-1.39);
// 	printf("rnet: delta = %5.3f\n",delta);

	// Kin is the shortwave incoming radiation
	Kin	= 1367.0 * (cos(sunzangle*PI/180) * tsw / (ds*ds) );
//  	printf("rnet: Kin = %5.3f\n",Kin);
	// Lin is incoming longwave radiation
	Lin	= (e_atm) * 5.67 * pow(10,-8) * pow((tempk-dtair),4);
//  	printf("rnet: Lin = %5.3f\n",Lin);
	// Lout is surface grey body emission in Longwave spectrum
	Lout	= e0 * 5.67 * pow(10,-8) * pow(tempk,4);
//  	printf("rnet: Lout = %5.3f\n",Lout);
	// Lcorr is outgoing longwave radiation "reflected" by the emissivity
	Lcorr	= (1.0 - e0) * Lin;
//  	printf("rnet: Lcorr = %5.3f\n",Lcorr);
	result	= (1.0 - bbalb) * Kin + Lin - Lout - Lcorr  ;
// 	printf("rnet: result = %5.3f\n",result);

	return result;
}

// Soil Heat Flux
double g_0(double bbalb, double ndvi, double tempk, double rnet, double time, int roerink)
{
	double 	a, b, result;
	double r0_coef;

	if (time<=9.0||time>15.0){
		r0_coef = 1.1;
	} else if (time>9.0&&time<=11.0){
		r0_coef = 1.0;
	} else if (time>11.0&&time<=13.0){
		r0_coef = 0.9;
	} else if (time>13.0&&time<=15.0){
		r0_coef = 1.0;
	}
	a = (0.0032 * (bbalb/r0_coef) + 0.0062 * (bbalb/r0_coef) * (bbalb/r0_coef));
	b = (1 - 0.978 * pow(ndvi,4));

	// Spain (Bastiaanssen, 1995)
	result = (rnet * (tempk-273.15) / bbalb) * a * b ;

	// HAPEX-Sahel (Roerink, 1995)
	if(roerink){
		result = result * 1.430 - 0.0845;
	}
	return result;
}


//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// DTAIR Initialization

/* Pixel-based input required are: tempk water & desert
 * additionally, dtair in Desert is vaguely initialized
 */
#define ZERO 273.15
double dt_air_0(double t0_dem, double tempk_water, double tempk_desert)
{
	double a, b, result;
	double dtair_desert_0;

	if(tempk_desert > (ZERO+48.0)){
		dtair_desert_0 = 13.0;
	} else if(tempk_desert >= (ZERO+40.0) && tempk_desert < (ZERO+48.0)){
		dtair_desert_0 = 10.0;
	} else if(tempk_desert >= (ZERO+32.0) && tempk_desert < (ZERO+40.0)){
		dtair_desert_0 = 7.0;
	} else if(tempk_desert >= (ZERO+25.0) && tempk_desert < (ZERO+32.0)){
		dtair_desert_0 = 5.0;
	} else if(tempk_desert >= (ZERO+18.0) && tempk_desert < (ZERO+25.0)){
		dtair_desert_0 = 3.0;
	} else if(tempk_desert >= (ZERO+11.0) && tempk_desert < (ZERO+18.0)){
		dtair_desert_0 = 1.0;
	} else {
		dtair_desert_0 = 0.0;
//		printf("WARNING!!! dtair_desert_0 is NOT VALID!\n");
	}
//	printf("dtair0 = %.0f K\t",dtair_desert_0);
	a = (dtair_desert_0-0.0)/(tempk_desert-tempk_water);
	b = 0.0 - a * tempk_water;
//	printf("dt_air_0(a) = %5.3f Tempk(b) %5.3f\n",a,b);
	result = t0_dem * a + b;
//	printf("dt_air_0 = %5.3f\n",result);
	return result;
}

// DTAIR Standard equation

/* Pixel-based input required are: tempk water & desert
 * additionally, dtair in Desert point should be given
 */
double dt_air(double t0_dem, double tempk_water, double tempk_desert, double dtair_desert)
{
	double a, b, result;
	a = (dtair_desert-0.0)/(tempk_desert-tempk_water);
	b = 0.0 - a * tempk_water;
	result = t0_dem * a + b;
	return result;
}

// DTAIR Dry Pixel
double dt_air_desert(double h_desert, double roh_air_desert, double rah_desert)
{
	double result;
	result = (h_desert * rah_desert)/(roh_air_desert * 1004);
	return result;
}

// Sensible Heat Flux Standard Equation
double h_0(double roh_air, double rah, double dtair)
{
	if(rah<0.01) rah=0.01;
	return(roh_air*1004*dtair / rah);
}

// Psichrometric parameter for heat momentum
double psi_h(double t0_dem, double h, double U_0, double roh_air)
{
	double most_L, most_x, psi_h;
//	printf("h input to psih() is %5.3f\n",h);
	if(h != 0.0){
		most_L = (-1004* roh_air*pow(U_0,3)* t0_dem)/(0.41*9.81* h);
	} else {
		most_L = -1000.0;
	}
	if(most_L < 0.0){
		most_x = pow((1-16*(2/most_L)),0.25);
		psi_h = (2*log((1+pow(most_x,2))/2));
	} else {
		most_x = 1.0;
		psi_h = -5*2/most_L;
	}
	return (psi_h);
}

// Aerodynamic resistance to heat momentum initialization

double rah_0(double zom_0, double u_0)
{
	return(log(200/zom_0)*log(2/(0.01*zom_0))/(u_0*0.41*0.41));
}

// Aerodynamic resistance to heat momentum standard equation

double rah1(double psih, double ustar, double zom_0)
{
	return((log(2/(0.01*zom_0))-psih)/(ustar*0.41));
}

// Air density Initialization

double roh_air_0(double tempk, int doy)
{
	double A, B, result;
	//MDBA Farm A
	double phase_max=sin(2*PI*(doy+365/3.3)/365);
	double tmax=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_max);
	double phase_min=sin(2*PI*(doy+365/3.5)/365);
	double tmin=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_min);
/*	A = (tmin*(6.11*exp(17.27*tmax/(tmax+237.3)))/100.0);
	B = (tmin*(6.11*exp(17.27*tmax/(tmax+237.3)))/100.0);*/
	A = (18*(6.11*exp(17.27*34.8/(34.8+237.3)))/100.0);
	B = (18*(6.11*exp(17.27*34.8/(34.8+237.3)))/100.0);
	result = (1000.0 - A)/(tempk*2.87)+ B/(tempk*4.61);
	return result;
}

// Air density standard paramterization

double rohair(double dem, double tempk, double dtair)
{
	double a, b, result;
	a = tempk - dtair;
	b = (( a - 0.00627*dem)/a);
	result = 349.467 * pow( b , 5.26)/ a ;
// 	printf("%5.2f ", result);
	return result;
}

// Wind Speed Initialization

double U_0(double zom_0, double u2m)
{
	double u_0;
	u_0=u2m*0.41*log(200/(1.0/7))/(log(2/(1.0/7))*log(200/zom_0));
//	printf("u_0 = %5.3f\n", u_0);
	return u_0;
}

// Nominal Wind Speed

double u_star(double t0_dem, double h, double ustar, double roh_air, double zom, double u2m)
{
	double result;
	double n5_temp; /* Monin-Obukov Length 		*/
        double n10_mem; /* psi m 			*/
	double n31_mem; /* x for U200 (that is bh...) 	*/
	double hv=1.0;	/* crop height (m) 		*/
	double bh=200;	/* blending height (m) 		*/

	if(h != 0.0){
		n5_temp = (-1004* roh_air*pow(ustar,3)* t0_dem)/(0.41*9.81* h);
	} else {
		n5_temp = -1000.0;
	}
	if(n5_temp < 0.0){
		n31_mem = pow((1-16*(200/n5_temp)),0.25);
		n10_mem = (2*log((1+n31_mem)/2)+log((1+pow(n31_mem,2))/2)-2*atan(n31_mem)+0.5*PI);
	} else {
//		n31_mem = 1.0;
		n10_mem = -5*2/n5_temp;
	}
	result = ((u2m*0.41/log(2/(hv/7)))/0.41*log(bh /(hv/7))*0.41)/(log(bh / zom)-n10_mem);
	return result;
}

// Roughness length for heat momentum

double zom_0(double ndvi, double ndvi_max)
{
	double a, b, zom;
	double hv_ndvimax=1.0; /* crop vegetation height (m) */
	double hv_desert=0.0002; /* desert base vegetation height (m) */
	a = (log(hv_desert)-((log(hv_ndvimax/7)-log(hv_desert))/(ndvi_max-0.02)*0.002));
	b = (log(hv_ndvimax/7)-log(hv_desert))/(ndvi_max-0.02);
	zom = exp(a+b*ndvi);
// 	printf("zom = %5.3f\n", zom);
	return zom;
}

// Sensible Heat flux determination

/* This is the main loop used in SEBAL */
/* Arrays Declarations */
#define ITER_MAX 100

double sensi_h( int iteration, double t0_dem, double ndvi, double ndvi_max, double dem, double rnet_desert, double g0_desert, double t0_dem_desert, double t0_dem_water, double u2m, double dem_desert, int doy, double *dt)
{
	/* Arrays Declarations */
	double dtair[ITER_MAX+1], roh_air[ITER_MAX+1], rah[ITER_MAX+1];
	double h[ITER_MAX+1];
	double ustar[ITER_MAX+1];

	/* Declarations */
	int	ic, debug=0;
	double	u_0, zom0;
	double	rah_desert;

	double	psih_desert,ustar_desertold;
	double	psih;

	/* Fat-free junk food */
	if (iteration>ITER_MAX){
		iteration=ITER_MAX;
	}
	double h_desert		= rnet_desert - g0_desert;
	double dtair_desert	= 0.2*h_desert/u2m;
	double roh_air_desert	= rohair(dem_desert,t0_dem_desert,dtair_desert);
	double zom_desert	= 0.002;
	double ustar_desert	= u2m*0.41/log(1000);
// 	printf("*************t0_dem=%f\tt0demwater=%f\tt0demdesert=%f\tdtairdesert=%f\t",t0_dem, t0_dem_water, t0_dem_desert, dtair_desert);
	dtair[0]	= dt_air(t0_dem, t0_dem_water, t0_dem_desert, dtair_desert);
// 	printf("*****************************dtair = %5.3f\n",dtair[0]);
// 	roh_air[0] 	= roh_air_0(t0_dem, doy);
	roh_air[0] 	= rohair(dem,t0_dem,dtair[0]);
// 	printf("*****************************rohair=%5.3f\n",roh_air[0]);
	zom0 		= zom_0(ndvi, ndvi_max);
// 	printf("*****************************zom = %5.3f\n",zom0);
	u_0 		= u2m;
// 	printf("*****************************u0 = %5.3f\n", u_0);
	rah[0] 		= rah_0(zom0, u_0);
// 	printf("*****************************rah = %5.3f\n",rah[0]);
	h[0] 		= h_0(roh_air[0], rah[0], dtair[0]);
// 	printf("*****************************h = %5.3f\n",h[0]);
	if(debug==1){
		printf("*****************************\n");
		printf("t0_dem = %5.3f\n",t0_dem);
		printf("ndvi = %5.3f ndvimax = %5.3f\n",ndvi,ndvi_max);
		printf("*****************************\n");
		printf("dtair[0]	= %5.3f K\n", dtair[0]);
		printf("roh_air[0] 	= %5.3f kg/m3\n", roh_air[0]);
		printf("dtair_desert 	= %8.5f\n", dtair_desert);
		printf("roh_air_desert0 = %5.3f kg/m3\n", roh_air_desert);
		printf("h_desert 	= %5.3f\n", h_desert);
		printf("zom_0 		= %5.3f\n", zom0);
		printf("u_0 		= %5.3f\n", u_0);
		printf("rah[0] 		= %5.3f s/m\n", rah[0]);
		printf("h[0] 		= %5.3f W/m2\n", h[0]);
	}

/*----------------------------------------------------------------*/
/*Main iteration loop of SEBAL*/
	for(ic=1;ic<iteration+1;ic++){
// 		psih = psi_h(t0_dem,h[ic-1],u_0,roh_air[ic-1]);
// 		ustar[ic] = u_star(t0_dem,h[ic-1],u_0,roh_air[ic-1],zom0,u2m);
		psih = psi_h(t0_dem,h[0],u_0,roh_air[ic-1]);
		ustar[ic] = u_star(t0_dem,h[0],u_0,roh_air[ic-1],zom0,u2m);
		rah[ic] = rah1(psih, ustar[ic],zom0);
		/* get desert point values from maps */
		roh_air_desert	= rohair(dem_desert,t0_dem_desert,dtair_desert);
		h_desert	= h_0(roh_air_desert,rah_desert,dtair_desert);
		if(h_desert<=0.0) h_desert=rnet_desert;
		if(h_desert>rnet_desert) h_desert=rnet_desert;
		ustar_desertold = ustar_desert;
		psih_desert 	= psi_h(t0_dem_desert,h_desert,ustar_desertold,roh_air_desert);
		ustar_desert	= u_star(t0_dem_desert,h_desert,ustar_desertold,roh_air_desert,zom_desert,u2m);
		rah_desert	= rah1(psih_desert,ustar_desert,zom_desert);
		dtair_desert 	= dt_air_desert(h_desert, roh_air_desert, rah_desert);
		if(dtair_desert<=1.0||dtair_desert>50.0) dtair_desert = 0.2*(rnet_desert - g0_desert)/u2m;
		/* This should find the new dtair from inversed h equation...*/
// 		printf("*************t0_dem=%f\tt0demwater=%f\tt0demdesert=%f\tdtairdesert=%f\t",t0_dem,t0_dem_water, t0_dem_desert, dtair_desert);
		dtair[ic] 	= dt_air(t0_dem, t0_dem_water, t0_dem_desert, dtair_desert);
// 		printf("*****************************dtair[%i] = %5.3f\n",ic,dtair[0]);
		/* This produces h[ic] and roh_air[ic+1] */
		roh_air[ic] 	= rohair(dem, t0_dem, dtair[ic]);
		h[ic] 		= h_0(roh_air[ic], rah[ic], dtair[ic]);
		if(h[ic]<=0.0) h[ic]=0;
		if(h[ic]>rnet_desert) h[ic]=rnet_desert;
		/* Output values of the iteration parameters */
		if(debug==1){
			printf("\n ******** ITERATION %i *********\n",ic);
			printf("psih[%i] 	= %5.3f\n", ic, psih);
			printf("ustar[%i] 	= %5.3f\n", ic, ustar[ic]);
			printf("rah[%i]		= %5.3f s/m\n",ic, rah[ic]);
			printf("rohair_desert	= %5.3f\n", roh_air_desert);
			printf("h_desert 	= %5.3f\n", h_desert);
			printf("psih_desert 	= %5.3f\tustar_desert = %5.3f\trah_desert = %5.3f\n", psih_desert, ustar_desert, rah_desert);
			printf("dtair_desert 	= %8.5f\n", dtair_desert);
			printf("dtair[%i] 	= %5.3f K\n", ic, dtair[ic]);
			printf("roh_air[%i] 	= %5.3f kg/m3\n", ic, roh_air[ic]);
			printf("h[%i] 		= %5.3f W/m2\n",ic, h[ic]);
		}
	}
	*dt=dtair[iteration];
	return h[iteration];
}

//Evaporative Fraction
double evap_fr(double r_net, double g0, double h0)
{
	return ((r_net - g0 - h0) / (r_net - g0));
}

// soil moisture in the root zone
// Makin, Molden and Bastiaanssen, 2001
double soilmoisture( double evap_fr )
{
	return ((exp((evap_fr-1.2836)/0.4213))/0.511);
}

// Evapotranspiration from energy balance

double et_a(double r_net_day, double evap_fr, double tempk)
{
	double latent, t_celsius;
	t_celsius = tempk - 273.15;
	latent 	  = 86400/((2.501-0.002361*t_celsius)*pow(10,6));
	return (r_net_day * evap_fr * latent);
}
