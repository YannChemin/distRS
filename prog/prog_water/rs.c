#include<stdio.h>
#include<math.h>
#define PI 3.1415927

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

// WATER
int water_modis(double surf_ref_7, double ndvi)
{
	double result;
	if (surf_ref_7<0.04&&ndvi<0.1){
		result = 1 ;
	} else {
		result = 0 ;
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
	double ws, cosun, latrad, delta, deltarad, ds, result;

	//Sun-Earth Distance (ds; A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
	//Solar declination (delta; radians)
	deltarad = 0.4093*sin((2*PI*doy/365)-1.39);
	//Convert latitude in radians
	latrad =  lat * PI / 180.0;
	
	//Convert latitude in radians
	ws = acos(-tan(latrad)*tan(deltarad));
	cosun = ws*sin(deltarad)*sin(latrad)+cos(deltarad)*cos(latrad)*sin(ws);
	
	result = ( cosun * 1367 * tsw ) / ( PI * ds * ds );

	return result;

}

// Average Diurnal Net Radiation after Bastiaanssen (1995)

double r_net_day( double bbalb, double solar, double tsw )
{
	double result;

	result = ((1.0 - bbalb)*solar)-(110.0*tsw);

	return result;
}

// Average Diurnal Potential ET after Bastiaanssen (1995)

double et_pot_day( double rnetd, double tempk, double roh_w )
{
	double latent, result;

	latent=(2.501-(0.002361*(tempk-273.15)))*1000000.0;
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
	double 	temp=0.0, ds=0.0, e_atm=0.0, delta=0.0;
	double	tsw_for_e_atm=0.7; //Special tsw, consider it a coefficient

// 	printf("**rnet: bbalb = %5.3f\n\tndvi = %5.3f\n\ttempk = %5.3f\n\te0 = %5.3f\n\ttsw = %5.3f\n\tdoy = %f\n\tutc = %5.3f\n\tsunzangle = %5.3f\n\tdtair = %5.3f\n",bbalb,ndvi,tempk,e0,tsw,doy,utc,sunzangle,dtair);
	
	// Atmospheric emissivity (Bastiaanssen, 1995)
	e_atm	=  1.08 * pow(-log10(tsw_for_e_atm),0.265) ;
	// Atmospheric emissivity (Pawan, 2004)
// 	e_atm	= 0.85 * pow(-log10(tsw),0.09);
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


