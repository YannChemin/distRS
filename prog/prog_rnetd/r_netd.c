 // Daily net radiation (W/m2)
#include<stdlib.h>
#include<math.h>
#include "r_netd.h"
#define PI 3.1415927

// Average Solar Diurnal Radiation after Bastiaanssen (1995)
double solar_day(double lat, double doy, double dem )
{
	double tsw, ws, cosun, latrad, deltarad, ds;
	double	Kexo;
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
// 	tsw = 0.75 + 2*pow(10,-5)*dem;
	tsw = 0.7;
// 	printf("tsw=%f\n", tsw);
	/* Kexo = exoatmospheric solar irradiance*/
	Kexo = k_exo(doy);
// 	printf("Kexo=%f\n", Kexo);
	//Sun-Earth Distance (ds; A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
// 	printf("ds=%f\n", ds);
	//Convert latitude in radians
// 	printf("lat=%f\n", lat);
	latrad =  lat * PI / 180.0;
// 	printf("latrad=%f\n", latrad);
	//Solar declination (delta; radians)
// 	deltarad = (0.4093*sin((2*PI*doy/365)-1.39))*(lat/fabs(lat));
	deltarad = (0.4093*sin((2*PI*doy/365)-1.39));
// 	printf("deltarad=%f\t", deltarad*180.0/PI);
	//Solar angle hour
// 	ws = (acos(-tan(latrad)*tan(deltarad)))+PI/2;
	ws = (acos(-tan(latrad)*tan(deltarad)));
// 	printf("ws=%f\t", ws);
	cosun = sin(deltarad)*sin(latrad)+cos(deltarad)*cos(latrad)*sin(ws);
// 	printf("cosun=%f\n", cosun);
// 	printf("solard=%f\n", ( cosun * Kexo * tsw ) / ( PI * ds * ds ));
// 	return ( ws*fabs(cosun) * Kexo * tsw ) / ( PI * ds * ds );
	return ( ws * cosun * Kexo * tsw ) / ( PI * ds * ds );
}

double k_exo(double doy)
{
	double Kexo,kexo_mean,r_prime_es,phi,ds;
	// #mean Earth-Sun distance in (m)
	r_prime_es=1.495978706916*pow(10,11);
	// #Total flux from the solar surface (W)
	phi=3.9*pow(10,26);
	// #mean Earth-Sun distance Kexo
	kexo_mean=phi/(4*PI*pow(r_prime_es,2));
	// #inverse relative distance Earth-Sun (A.U.)
	ds = 1.0 + 0.01672 * sin(2*PI*(doy-93.5)/365.0);
	// #kexo on doy date
	Kexo=kexo_mean*(pow(r_prime_es,2))/(pow(r_prime_es/ds,2));
	return Kexo;
}

// Average Diurnal Net Radiation initially after Bastiaanssen (1995)
double r_net_day( double bbalb, double solar, double dem )
{
	double tsw;
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
// 	tsw = 0.75 + 2*pow(10,-5)*dem;
	tsw = 0.7;
// 	printf("rnetd=%f\n", ((1.0 - bbalb)*solar)-(110.0*tsw));
	return ((1.0 - bbalb)*solar)-(110.0*tsw);
}

// Average Diurnal Net Radiation initially after Chemin and Ahmad (2000)
double r_net_d( double bbalb, double solar, double e0, double tempka, double dem )
{
	double result,tsw, eatm;
// 	double  L24in, B24out;
// 	B24out = 5.67*pow(10,-8)*pow(tempka,4);
	/* tsw =  atmospheric transmissivity single-way (~0.7 -) */
// 	tsw = 0.75 + 2*pow(10,-5)*dem;
// 	tsw = 0.68;
	tsw = 0.7;
//	#ea = Actual Vapour pressure [hPa]
	//Bastiaanssen, 1995
	eatm = 1.08*pow(-log(tsw),0.265);
// 	eatm = 1.34-(0.14*sqrt(ea));
// 	L24in = eatm*5.67*pow(10,-8)*pow(tempka,4);
	result = ((1.0 - bbalb)*solar)+e0*5.67*pow(10,-8)*pow(tempka,4)*(eatm-1);
// 	result = ((1.0 - bbalb)*solar)+L24in-(e0*B24out)-(1-e0)*L24in;
	return result;
}
