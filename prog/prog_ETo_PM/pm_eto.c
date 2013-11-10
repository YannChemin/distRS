// PENMAN-MONTEITH ETo
#include<math.h>
#include "pm_eto.h"
#define PI 3.1415927

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

double EToPM(double T, double Z, double u2, double Rn, double Rh, double hc)
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
    ETrad = (delta / (delta + gstar)) * ((Rn - G) / L);
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


