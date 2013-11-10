#include<stdio.h>
double idx(double min, double max, double pixel);
// NDVI
double ndvi(double red, double nir);
// WATER
int water_modis(double surf_ref_7, double ndvi);

// Broadband albedo MODIS

double bb_alb_modis( double redchan, double nirchan, double chan3, double chan4, double chan5, double chan6, double chan7 );

double deltat_wim(double tempk);

// Average Solar Diurnal Radiation after Bastiaanssen (1995) 
double solar_day(double lat, double doy, double tsw );

// Average Diurnal Net Radiation after Bastiaanssen (1995)
double r_net_day( double bbalb, double solar, double tsw );

// Average Diurnal Potential ET after Bastiaanssen (1995)
double et_pot_day( double rnetd, double tempk, double roh_w );

// Instantaneous net radiation (W/m2)
double r_net( double bbalb, double ndvi, double tempk, double dtair,  double e0, double tsw, double doy, double utc, double sunzangle );

// Soil Heat Flux
double g_0(double bbalb, double ndvi, double tempk, double rnet, double time, int roerink);

double dt_air_0(double t0_dem, double tempk_water, double tempk_desert);

// DTAIR Standard equation
double dt_air(double t0_dem, double tempk_water, double tempk_desert, double dtair_desert);

// DTAIR Dry Pixel
double dt_air_desert(double h_desert, double roh_air_desert, double rah_desert);

// Sensible Heat Flux Standard Equation
double h_0(double roh_air, double rah, double dtair);

// Sensible Heat Flux Standard Equation
double h1(double roh_air, double rah, double dtair);

// Psichrometric parameter for heat momentum
double psi_h(double t0_dem, double h, double U_0, double roh_air);

// Aerodynamic resistance to heat momentum initialization
double rah_0(double zom_0, double u_0);

// Aerodynamic resistance to heat momentum standard equation
double rah1(double psih, double ustar, double zom_0);

// Air density Initialization
double roh_air_0(double tempk, int doy);

// Air density standard paramterization
double rohair(double dem, double tempk, double dtair);

// Wind Speed Initialization
double U_0(double zom_0, double u2m);

// Nominal Wind Speed
double u_star(double t0_dem, double h, double ustar, double roh_air, double zom, double u2m);

// Roughness length for heat momentum 
double zom_0(double ndvi, double ndvi_max);

// Sensible Heat flux determination
double sensi_h( int iteration, double t0_dem, double ndvi, double ndvi_max, double dem, double rnet_desert, double g0_desert, double t0_dem_desert, double t0_dem_water, double u2m, double dem_desert, int doy, double *dt);


//Evaporative Fraction
double evap_fr(double r_net, double g0, double h0);

// soil moisture in the root zone
// Makin, Molden and Bastiaanssen, 2001
double soilmoisture( double evap_fr );

// Evapotranspiration from energy balance
double et_a(double r_net_day, double evap_fr, double tempk);
