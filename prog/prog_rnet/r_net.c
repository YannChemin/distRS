// Instantaneous net radiation (W/m2)
#include<math.h>
#include "r_net.h"
#define PI 3.1415927
/* dem is input to tsw. tsw is input to eatm*/
double r_net( double albedo, double sunza, double emissivity, double lst, double dem, double doy, double tmax)
{
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
	/* doy = Day of Year */
	/* sunza = sun zenith angle at sat. overpass */
	/* tmax = max air temperature (approximative, or met.station) */
	double result;
	double sb; /*Stephan-boltzman Constant*/
	double ds; /*relative sun-earth distance in astronomical units*/
	double tsw;/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
	double kexo;/* Kexo = exoatmospheric solar irradiance*/
	double eatm; /*Eatm Atmospheric emissivity [-]*/
	ds = 1-0.01672*cos(2*PI*(doy-4)/365.0);
	sb = 5.67 * pow(10,-8);
// 	tsw = 0.75 + 2*pow(10,-5)*dem;
// 	tsw = 0.68;
	tsw = 0.7;
	//Bastiaanssen, 1995
 	eatm = 1.08*pow(-log(tsw),0.265);
	//Pawan, 2004
//	eatm = 0.85*pow(-log(tsw),0.09);
	kexo = k_exo(doy);
// 	kexo = 1358;
	result=(1-albedo)*(tsw*kexo*cos(sunza*PI/180.0)/(ds*ds))+(eatm*sb*pow(tmax,4))-(emissivity*sb*pow(lst,4))-(1-emissivity)*(eatm*sb*pow(tmax,4));
	return result;
}

double k_exo(double doy)
{
	double Kexo,kexo_mean,r_prime_es,phi,dr;
	// #mean Earth-Sun distance in (m)
	r_prime_es=1.495978706916*pow(10,11);
	// #Total flux from the solar surface (W)
	phi=3.9*pow(10,26);
	// #mean Earth-Sun distance Kexo
	kexo_mean=phi/(4*PI*pow(r_prime_es,2));
	// #inverse relative distance Earth-Sun (A.U.)
	dr=1+0.033*cos(2*PI*doy/365);
	// #kexo on doy date
	Kexo=kexo_mean*(pow(r_prime_es,2))/(pow(dr*r_prime_es,2));
	return Kexo;
}