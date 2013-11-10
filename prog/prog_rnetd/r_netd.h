#include<stdio.h>

// Daily net radiation (W/m2)
double solar_day( double lat, double doy, double dem );
double k_exo( double doy );
double r_net_day( double bbalb, double solar, double dem );
double r_net_d( double bbalb, double solar, double e0, double tempka, double dem );

