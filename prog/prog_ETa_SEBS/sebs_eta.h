#include<stdio.h>

//SEBS Zu 1999
double kb_1(double u_zref,double zref,double h,double lai,double wfol,double tempka,double pa);

double psim(double y);
double psih(double y);
double bw(double hi, double molength, double z0);
double cw(double hi, double molength, double z0, double z0h);
double e_sat(double lst, double dem);
double eact(double esat_avg, double hr_s);

double k_exo(double doy);
double k_in( double dem, double doy, double sunza);
// Average Solar Diurnal Radiation after Bastiaanssen (1995)
double k_in24(double latitude, double doy, double dem );

double sebs_evapfr(double z_pbl, double t_s, double p_s, double u_s, double hr_s, double alt_ms, double eact, double kin, double albedo, double ndvi, double ndvi_min, double ndvi_max, double ndvi_median, double e0, double lst, double dem);

double sebs_eta(double doy, double ndvi, double ndvi_max, double ndvi_min, double ndvi_median, double u_s, double z_pbl, double t_s, double p_s, double alt_ms, double e_act, double latitude, double kin24, double albedo, double dem, double lst, double e0, double evapfr);