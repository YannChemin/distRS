// SSEB ETa (Senay, 2007)
#include<math.h>
#include "sseb_eta.h"

//SSEB Senay G.B., Budde M., Verdin J.P., and Melesse A.M., 2007, Sensors, 7:979-1000.
double sseb_evapfr(double lst_h,double lst_c,double lst) //Lst hot, Lst cold, Lst, 
{
	return((lst_h-lst)/(lst_h-lst_c));
}


// soil moisture in the root zone
// Makin, Molden and Bastiaanssen, 2001
double soilmoisture( double evap_fr )
{ 
	return ((exp((evap_fr-1.2836)/0.4213))/0.511);
}