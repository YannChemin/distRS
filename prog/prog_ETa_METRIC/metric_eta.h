#include<stdio.h>

// METRIC ETa (Allen, 2005)
double Rsky(double ta);
/* Narrow band emissivity calculations (e_NB) */
double e_NB(double ndvi,double lai);
/* Surface temperature calculations Landsat 5TM */
double T0L5(double e_NB,double L6rad,double Rsky);
// Surface emissivity calculations (e_0)
double e_0(double ndvi,double lai);
//Outgoing longwave radiation (Lout)
double Lout(double e_0,double t0);
//Incoming longwave radiation (Lin). To be derieved locally at CSU
double Lin(double tsw,double ta);
//Net longwave radiation (Lnet)
double Lnet(double Lout,double Lin);
//Solar shortwave incoming irradiance
double Kin(double doy, double lat, double tsw);
//Net Radiation (Rnet)
double Rnet(double alb,double Lnet,double Kin);
//calculation of g0
double g0(double Rnet,double t0,double alb,double ndvi);
//Calculation of surface roughness of momentum
double z0m(double lai);
//Calculation of effitive wind speed (initialsation of a unique point based ustar)
double ustar0(double u,double z,double h);
//Calculation of aerodynamic resistance to heat flux momentum
double rah0(double ustar0);//initialsation of rah calculation
//INSIDE dTair
double _rohair(double dem,double t0,double dT);
// calculation of dTair (spreadsheet calculations)
double dTair(double a[10],double b[10],double eto_alf,double kc,double dem_wet,double
t0_wet,double Rnet_wet,double g0_wet,double dem_dry,double t0_dry,double
Rnet_dry,double g0_dry);
//calculation for air density
double rohair(double dem,double t0,double dTair);
//calculation of sensible heat flux (Single equation)
double h(double rohair,double dTair,double rah);
//Monin-Obukov Length calculation
double L(double rohair,double ustar,double t0,double h);
// psychrometric momentum constant calculation
double psim200(double L);
// psychrometric heat constant calculation
double psih2(double L);
// psychrometric heat constant calculation
double psih01(double L);
//Calculation of effitive wind speed
double ustar2(double u200,double z0m,double psim200);
//Calculation of aerodynamic resistance to heat flux momentum
double rah(double ustar,double psih2,double psih01); //initialsation of rah calculation
//Sensible Heat flux Calculations
double metiter(double a[10], double b[10],double t0,double rah0,double z0m,double
ustar0, double dem, double u200, int iteration);
//ETinst
double ETinst(double Rnet,double g0,double h0,double t0);
//Daily_N
double daily_N(double doy, double lat);
//ETo PM
double EToPM(double T, double Z, double u2, double Rn, double Rh, double hc, double daily_N);

// soil moisture in the root zone
// Makin, Molden and Bastiaanssen, 2001
double soilmoisture( double evap_fr );