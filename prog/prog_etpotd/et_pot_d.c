//Average Diurnal Potential ET after Bastiaanssen (1995)
#include<math.h>
#include "et_pot_d.h"
#define PI 3.1415927

//Average Diurnal Potential ET after Bastiaanssen (1995)
double et_pot_d(double rnetd, double tempk, double roh_w )
{
	double latent=(2.501-(0.02361*(tempk-273.15)))*1000000.0;
	return((rnetd*86400*1000.0)/(latent*roh_w));
}
