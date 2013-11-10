#include<stdio.h>

// Instantaneous net radiation (W/m2)
double r_net( double albedo, double sunza, double emissivity, double lst, double dem, double doy, double tmax );
double k_exo(double doy);