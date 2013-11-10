// METRIC ETa (Allen, 2005)
#include<math.h>
#include "metric_eta.h"
#define PI 3.1415927

/* METRIC */
/* ET - Energy Balance Modeling */

/* NON-SPATIAL FUNCTIONS */

double Rsky(double ta) /*ta is near surface airtemperature from meteorological
data*/
{
	return(1.807 * pow(10,-10) * pow(ta,4) * (1 - 0.26 * exp(-7.77 *
pow(10,-4) * pow((273.15 - ta),2))));
}

/* SPATIAL FUNCTIONS */
/* Narrow band emissivity calculations (e_NB) */
double e_NB(double ndvi,double lai)
{
	if(ndvi > 0 && ndvi < 3) return(0.97 + 0.0033 * lai);
	else if(lai >= 3) return(0.98);
	else return(0.99);
}
/* Surface temperature calculations Landsat 5TM */
double T0L5(double e_NB,double L6rad,double Rsky)
{
	/*R_sky = 1.4199*/ /*Spreadsheet calculations, based on air temperature.
Page 26 Metric manual*/
// 	double T_NB = 0.866 ; /*Page 26 Metric manual*/
// 	double RP = 0.91 ; /*Page 26 Metric manual*/
	return(1260.56/log(1 + (e_NB * 607.76) / L6rad));
}

// Surface emissivity calculations (e_0)
double e_0(double ndvi,double lai)
{
	if(ndvi > 0 && lai < 3) return(0.95 + 0.01 * lai);
	else if(lai >= 3) return(0.98);
	else return(0.985);
}

//Outgoing longwave radiation (Lout)
double Lout(double e_0,double t0)
{
	return(5.67 * pow(10,-8) * pow(t0,4));
}

//Incoming longwave radiation (Lin). To be derieved locally at CSU
double Lin(double tsw,double ta)
{
	return(0.85 * pow(-log(tsw),0.09) * 5.67 * pow(10,-8) * pow(ta,4));
}

//Net longwave radiation (Lnet)
double Lnet(double Lout,double Lin)
{
	return(Lin - Lout);
}

//Solar shortwave incoming irradiance
double Kin(double doy, double lat, double tsw)
{
	double dr, ra, latrad, deltarad, ws;
	dr = 1 + 0.033 * cos(2*PI/365.0*doy);
	latrad = lat*PI/180.0;
	deltarad = 0.409 * sin(2*PI/365*doy-1.39);
	ws = acos(-tan(latrad)*tan(deltarad));
// 	ra = 24/PI*1366.6*dr*(ws*sin(latrad)*sin(deltarad)+cos(latrad)*cos(deltarad)*sin(ws));
	ra = 1366.6*dr*(ws*sin(latrad)*sin(deltarad)+cos(latrad)*cos(deltarad)*sin(ws));
// 	printf("ra=%f [W/m2]\n", ra);
	double result=tsw*ra;
// 	printf("KIN=%f [W/m2]\n", result);
	return(result);
}

//Net Radiation (Rnet)
double Rnet(double alb,double Lnet,double Kin)
{
	//Kin from speadsheets
	double result=(1 - alb) * Kin + Lnet;
	return(result);
}

//calculation of g0: T0 in Celsius !
double g0(double Rnet,double t0,double alb,double ndvi)
{
	double result=Rnet * (t0 - 273.15) * ( 0.0038 + 0.0074 * alb) * (1 - 0.98 *
pow(ndvi,4));
	return(result);
}

//Calculation of surface roughness of momentum
double z0m(double lai)
{
      return(0.018 * lai);
}

//Calculation of effitive wind speed (initialsation of a unique point based ustar)
double ustar0(double u,double z,double h)
{
      // u is wind speed at z meter, with vegetation height below of h meters
      //u200 calculated from equation 41, page 39
      double ustar = 0.41 * u / (log(z/(0.12*h)));
      double u200 = ustar * log (200/(0.12*h)) / 0.41;
      return(0.41 * u200 / log(200 / (0.12*h)));
}

//Calculation of aerodynamic resistance to heat flux momentum
double rah0(double ustar0) //initialsation of rah calculation
{
      return(log(2 / 0.1)/(ustar0 * 0.41));
}
//INSIDE dTair
double _rohair(double dem,double t0,double dT)
{
// 	return(1000 *patm / (1.01 * (t0-dT) * 287));
	return(349.467 * pow((((t0-dT)-0.0065 * dem)/(t0-dT)),5.26) / t0);
}

// calculation of dTair (spreadsheet calculations)
double dTair(double a[10],double b[10],double eto_alf,double kc,double dem_wet,double
t0_wet,double Rnet_wet,double g0_wet,double dem_dry,double t0_dry,double
Rnet_dry,double g0_dry)
{
	// Somehow T0_dem is not used in the spreadsheet
	// a is slope and b is offset
	int i;
	double eto=eto_alf;
	double LE_wet=eto*kc*(2.501-0.002361*(t0_wet-273.15))* pow(10,6)/3600;
	double LE_dry=0.0;
// 	double h_wet= 0;
	double h_wet= Rnet_wet-g0_wet-LE_wet;
	if(h_wet<0.0) h_wet=0.0;
	double h_dry= Rnet_dry-g0_dry-LE_dry;
	printf("h_wet=%f\th_dry=%f\n",h_wet,h_dry);
	double airden_wet=1000*101.3*pow((293-0.0065*dem_wet)/293,5.26)/(1.01*287*293);
	double airden_dry=1000*101.3*pow((293-0.0065*dem_dry)/293,5.26)/(1.01*287*293);
	printf("airden_wet=%f\tairden_dry=%f\n",airden_wet,airden_dry);
	double ustar_wet=ustar0(3.6,200,0.75);//common cereal crops average height
	double ustar_dry=ustar0(3.6,200,0.017);//standard desert sebal parameters
	printf("ustar_wet=%f\tustar_dry=%f\n",ustar_wet,ustar_dry);
	double rah_wet=rah0(ustar_wet);
	double rah_dry=rah0(ustar_dry);
	printf("rah_wet=%f\trah_dry=%f\n",rah_wet,rah_dry);
	double dT_wet=h_wet*rah_wet/(airden_wet*1004);
	double dT_dry=h_dry*rah_dry/(airden_dry*1004);
	printf("dT_wet=%f\tdT_dry=%f\n",dT_wet,dT_dry);
	a[0]=0.0;
	b[0]=0.0;
	a[0]=(dT_dry-dT_wet)/(t0_dry-t0_wet);
	b[0]= (-1.0 * a[0] * t0_wet) + dT_wet;
	printf("a[0]=%f\tb[0]=%f\n",a[0],b[0]);
	double L_wet, L_dry;
	double psim200_wet, psim200_dry;
	double psih2_wet, psih2_dry;
	double psih01_wet, psih01_dry;
	double z0m_wet, z0m_dry;

	//dT1 is here
	for (i=1;i<=8;i++) {
		a[i]=0.0;
		b[i]=0.0;
		airden_wet=_rohair(dem_wet,t0_wet,dT_wet);
		airden_dry=_rohair(dem_dry,t0_dry,dT_dry);
		h_wet=h(airden_wet,dT_wet,rah_wet);
		h_dry=h(airden_dry,dT_dry,rah_dry);
		if(h_wet==0) L_wet= -1000;
		else L_wet= -1004*airden_wet*pow(ustar_wet,3)*t0_wet/(0.41*h_wet*9.81);
		if(h_dry==0) L_dry= -1000;
		else L_dry= -1004*airden_dry*pow(ustar_dry,3)*t0_dry/(0.41*h_dry*9.81);
		psim200_wet=psim200(L_wet);
		psim200_dry=psim200(L_dry);
		psih2_wet=psih2(L_wet);
		psih2_dry=psih2(L_dry);
		psih01_wet=psih01(L_wet);
		psih01_dry=psih01(L_dry);
		z0m_wet=0.12*0.75;
		z0m_dry=0.12*0.02;
		ustar_wet=ustar2(3.6,z0m_wet,psim200_wet);
		ustar_dry=ustar2(3.6,z0m_dry,psim200_dry);
		rah_wet=rah(ustar_wet,psih2_wet,psih01_wet);
		rah_dry=rah(ustar_dry,psih2_dry,psih01_dry);
		dT_wet=h_wet*rah_wet/(airden_wet*1004);
		dT_dry=h_dry*rah_dry/(airden_dry*1004);
		a[i]=(dT_dry-dT_wet)/(t0_dry-t0_wet);
		b[i]= (-1.0 * a[i] * t0_wet) + dT_wet;
		printf("a[%d]=%f\tb[%d]=%f\n",i,a[i],i,b[i]);
	}
	return(0);
}

//calculation for air density
double rohair(double dem,double t0,double dTair)
{
	double patm=101.3 * pow((293 - 0.0065 * dem)/293, 5.26);
	return(patm * 1000 / (1.01 * (t0 - dTair) * 287));
}

//calculation of sensible heat flux (Single equation)
double h(double rohair,double dTair,double rah)
{
	return(rohair * 1004 * dTair / rah);
}

//Monin-Obukov Length calculation
double L(double rohair,double ustar,double t0,double h)
{
	return(rohair * 1004 * pow(ustar,3) * t0 / (0.41 * 9.807 * h));
}

// psychrometric momentum constant calculation
double psim200(double L)
{
	if (L < 0) {
		double x200=pow((1 - 16 * 200/L),0.25);
		return(2 * log((1 + x200)/2) + log((1 + pow(x200,2))/2) - 2 *
atan(x200 + 0.5 * PI));
	} else {
		return(-5 * 2 / L);
	}
}

// psychrometric heat constant calculation
double psih2(double L)
{
	if (L < 0) {
		double x2=pow((1 - 16 * 2/L),0.25);
		return(2 * log((1 + pow(x2,2))/2));
	} else {
		return(-5 * 2 / L);
	}
}

// psychrometric heat constant calculation
double psih01(double L)
{
	if (L < 0) {
		double x01=pow((1 - 16 * 0.1/L),0.25);
		return(2 * log((1 + pow(x01,2))/2));
	} else {
		return(-5 * 2 / L);
	}
}

//Calculation of effitive wind speed
double ustar2(double u200,double z0m,double psim200)
{
      //u200 calculated from equation 41, page 39
      return(0.41 * u200 / (log(200 / z0m) - psim200));
}

//Calculation of aerodynamic resistance to heat flux momentum
double rah(double ustar,double psih2,double psih01) //initialsation of rah calculation
{
      return((log(2 / 0.1) - psih2 + psih01) / (ustar * 0.41));
}

//Sensible Heat flux Calculations
double metiter(double a[10], double b[10],double t0,double rah0,double z0m,double
ustar0, double dem, double u200, int iteration)
{
	int i;
	double dT, airden, h0, L0, psim_200, psih_2, psih_01, u_star, r_ah;
	r_ah=rah0;
	for (i=0;i<iteration;i++) {
		dT 	= a[i] * t0 + b[i];
// 		printf("a[%i]=%f\tb[%i]=%f\n",i,a[i],i,b[i]);
		airden 	= rohair(dem,t0,dT);
		h0 	= h(airden,dT,r_ah);
		L0 	= L(airden,ustar0,t0,h0);
		psim_200= psim200(L0);
		psih_2 	= psih2(L0);
		psih_01 = psih01(L0);
		u_star 	= ustar2(u200,z0m,psim_200);
		r_ah 	= rah(psih_2,psih_01,u_star);
	}
	return(h0);
}

//ETinst
double ETinst(double Rnet,double g0,double h0,double t0)
{
	double LHF;
	if ((Rnet - g0 - h0)<=0){
		if ((Rnet - h0)>0){
			LHF = Rnet - h0;
		} else {
			LHF = Rnet;
		}
	} else {
		LHF = Rnet - g0 - h0;
	}
	return(3600 * LHF / ((2.501 - 0.00236 * (t0 - 273.15)) * 1000000));
}

double daily_N(double doy, double lat)
{
	double latrad = lat*PI/180.0;
	double deltarad = 0.409 * sin(2*PI/365*doy-1.39);
	double ws = acos(-tan(latrad)*tan(deltarad));
	return (24*ws/PI);
}

//ETo PM

/* constant definition */
/*#define k_sb 4.903    [MJ/m2*h] Stefan Bolzman constant */
#define cp 1.013		/*[kJ/kg*C] specific heat of moist air */
#define epsilon 0.622		/*[-]   ratio of molecular weigth of water to dry air */
#define Po 101.3		/*[kPa] atmospheric pressure at sea level */
#define Tko 293.16		/*[K]   reference temperature at sea level */
#define eta 0.0065		/*[K/m] constant lapse rate */
#define Ao 0			/*[m]   altitude at sea level */
#define g 9.81			/*[m/s] gravitational accelleration */
#define R 287			/*[J/kg*K] specific gas constant */
#define Zw 2			/*[m]   height of  wind measurements */
#define Zh 2			/*[m]   height of  humidity measurements */
#define k 0.41			/*[-]   Von Karman constant */

double EToPM(double T, double Z, double u2, double Rn, double Rh, double hc, double daily_N)
{
    double ea, delta, gamma, gstar, L;
    double P, ra, d, Zom, Zoh, G, ETrad, u10, rs, ed, Tkv, rho, ETaero, ETp;
    T -= 273.15;
    if(hc<0.001) hc=0.001;
//     printf("hc=%f\t",hc);
//     printf("Rn=%f\t",Rn);
    /* mean saturation vapour pressure [KPa] */
    ea = 0.61078 * exp((17.27 * T) / (T + 237.3));
//     printf("ea=%f\t",ea);
    /* slope of vapour pressure curve [KPa/C] */
    delta = (4098 * ea) / pow((237.3 + T), 2);
//     printf("delta=%f\t",delta);
    /* latent heat vapourisation [MJ/kg]  */
    L = 2.501 - (0.002361 * T);
//     printf("L=%f\t",L);
    /* atmospheric pressure [KPa] */
    P = Po * pow(((Tko - eta * (Z - Ao)) / Tko), (g / (eta * R)));
//     printf("P=%f\t",P);
    /* psichometric constant [kPa/C] */
    gamma = ((cp * P) / (epsilon * L)) * 0.001;
//     printf("gamma=%f\t",gamma);
    /* aerodynamic resistance [s/m] */
    if (hc < 2) {
	d = (2 / 3) * hc;
	Zom = 0.123 * hc;
	Zoh = 0.1 * Zom;
	ra = (log((Zw - d) / Zom) * log((Zh - d) / Zoh)) / (k * k * u2);
    } else {
	u10 = u2 * (log((67.8 * 10) - 5.42)) / 4.87;
	ra = 94 / u10;
    }
//     printf("ra=%f\t",ra);
    /* surface resistance [s/m]  */
    rs = 100 / (0.5 * 24 * hc);
//     printf("rs=%f\t",rs);
    /*modified psichometric constant [kPa/C] */
    gstar = gamma * (1 + (rs / ra));
    /*net radiation [MJ/m2*d] */
    /*Rn derived from r.sun */
    /*soil heat flux [MJ/m2*d] */
    G = 0.1 * Rn;
//     printf("G=%f\t",G);
    /* radiation term [mm/h] */
    /* ETrad = (delta/(delta+gstar))*((Rn-G)/(L*1000000)); */
    ETrad = (delta / (delta + gstar)) * ((Rn - G) / (L*daily_N));
//     printf("ETrad=%f\t",ETrad);
    /* actual vapour pressure [kPa] */
    ed = Rh * ea / 100;
//     printf("ed=%f\t",ed);
    /* virtual temperature [C] */
    Tkv = (T + 273.15) / (1 - (0.378 * ed / P));
//     printf("Tkv=%f\t",Tkv);
    /* atmospheric density [Kg/m^3] */
    rho = P / (Tkv * R / 100);
//     printf("rho=%f\t",rho);
    /* aerodynamic term [mm/h] */
//     ETaero = (0.001/L)*(1/(delta+gstar))*(rho*cp/ra)*(ea-ed);
    ETaero = (3.6 / L) * (1 / (delta + gstar)) * (rho * cp / ra) * (ea - ed);
//     printf("ETaero=%f\t",ETaero);
    /* potential evapotranspiration [mm/h] */
    ETp = ETrad + ETaero;
//     printf("ETp=%f\n",ETp);
    return(ETp);
}


// soil moisture in the root zone
// Makin, Molden and Bastiaanssen, 2001
double soilmoisture( double evap_fr )
{
	return ((exp((evap_fr-1.2836)/0.4213))/0.511);
}

