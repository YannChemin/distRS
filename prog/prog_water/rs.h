/********************************/
/* Prototypes for RS funktions 	*/
/********************************/
#include<stdio.h>

double ndvi(double red, double nir);
int water_modis(double surf_ref_7, double ndvi);
double bb_alb_modis( double redchan, double nirchan, double chan3, double chan4, double chan5, double chan6, double chan7 );
double deltat_wim(double tempk);
double solar_day(double lat, double doy, double tsw );
double r_net_day( double bbalb, double solar, double tsw );
double et_pot_day( double rnetd, double tempk, double roh_w );
double r_net( double bbalb, double ndvi, double tempk, double dtair,  double e0, double tsw, double doy, double utc, double sunzangle );
double g_0(double bbalb, double ndvi, double tempk, double rnet, double time, int roerink);
