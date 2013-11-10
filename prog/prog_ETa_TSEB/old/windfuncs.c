#include <stdio.h>
#include <math.h>
#include "tseb_eta.h"
double ustar_func(double K, double u, double Zu, double d0m,
                  double z0m,double Psim)
{
	double ustar;
	#if 0
	printf("in ustar_func:\n");
	printf("u: %f Zu: %f d0m: %f z0m: %f Psim: %f\n",u,Zu,d0m,z0m,Psim);
	#endif
	ustar = K*u/(log((Zu-d0m)/z0m)-Psim);
	/*  printf("ustar: %f\n",ustar);*/
	return(ustar);
}

double ucpy_func(double u, double hc, double d0m, double z0m, double K, double ustar)
{
	double ucanopy;
	ucanopy = u*((log((hc-d0m)/z0m)/(u*K/ustar)));
	return(ucanopy);
}

double usoil_func(double ucanopy,double a, double hc, double d0m, double z0m)
{
	double usoil;
	usoil = ucanopy*exp(a*(((z0m+d0m)/hc)-1.0));
	#if 0
	printf("usoil: %f\n",usoil);
	exit(1);
	#endif
	return(usoil);
}
double usoilbare_func(double Zref,double Zu,double d0m, double z0m,double Psimr,double Psim, double u)
{
	double numer,denom,usoil;
	numer = log((Zref-d0m)/z0m)-Psimr;
	denom = log((Zu-d0m)/z0m)-Psim;
	usoil = (numer/denom)*u;
	return(usoil);
}

/* wind_decay returns 'a' parameter for use inside canopy*/
/* to compute wind speed inside canopy*/
double wind_decay(double LAI,double Omega, double hc,double s)
{
	double p = 2.0/3.0;
	double q = 1.0/3.0;
	double a;
	a = 0.28*(pow((LAI*Omega),p))*(pow(hc,q))/(pow(s,q));
	return(a);
}

/*Dyer and Hicks stability function for heat*/
double phih_dyer(double zeta, double L)
{
	double phih;
	phih = pow((1.0-16.0*zeta),-0.5);
	return(phih);
}

/*Dyer and Hicks stability function for momentum*/
double phim_dyer(double zeta, double L)
{
	double phim;
	phim = pow((1.0-16.0*zeta),-0.25);
	return(phim);
}

double psim_func(double x)
{
	double psim;
	psim = 2.0*log((1.0+x)/2.0)+log((1.0+(x*x))/2.0)-
	  2.0*atan(x)+(M_PI/2.0);
	return(psim);
}

double psih_func(double x)
{
	double psih;
	psih = 2.0*log((1.0+(x*x))/2.0);
	return(psih);
}

double ra_func_orig(double Zt, double d0m, double z0m,double Psih, double U, double K, double Psim, double Zu)
{
	/* formula from Norman et al. 95 */
	double ra;
	double numer1,numer2,denom;
	double kB_1 = 2.0;
	double z0h;
	z0h = z0m/exp(kB_1);
	numer1 = log((Zu-d0m)/z0m)-Psim;
	numer2 = log((Zt-d0m)/z0h)-Psih;
	denom = K*K*U;
	ra = (numer1*numer2)/denom;
	#if 0
	printf("using ra_func_orig in windfuncs.h\n");
	printf("z0h: %f Zu: %f d0m: %f z0m: %f Psim: %f\n",z0h,Zu,d0m,z0m,Psim);
	printf("Zt: %f Psih: %f\n",Zt,Psih);
	printf("K: %f U: %f\n",K,U);
	printf("ra is: %f\n",ra);
	/* exit(1);*/
	#endif
	return(ra);
}

double ra_func(double Zt, double d0m, double z0m, double Psih, double ustar, double K)
{
	double ra;
	/*  printf("Zt: %f d0m: %f z0m: %f Psih: %f ustar: %f K: %f\n",*/
	/*	 Zt,d0m,z0m,Psih,ustar,K);*/
	ra = (log((Zt-d0m)*7.0/z0m)-Psih)/(ustar*K);
	return(ra);
	#if 0
	printf("using ra_func in windfuncs.h\n");
	printf("ra is: %f\n",ra);
	exit(1);
	#endif
}

/*compute ra for bare soil using Reynolds roughness */
double ra_funcbareRe(double Zt, double Zref, double d0m, double z0m,double Psih,double Psir,double ustar,double K, double rho, double tkair, double tkrad)
{
	double rabrRe;
	double Tas;
	double mu,mu0;
	/*mu0 is dynamic viscosity */
	double nu;
	double Restar;
	double z0h;
	Tas = 0.5*(tkair+tkrad);
	mu0 = 1.8325e-5;
	mu = ((296.16+393.16)/(Tas+393.16))*(pow((Tas/296.16),1.5))*mu0;
	nu = mu/rho;
	Restar = ustar*z0m/nu;
	z0h = z0m*exp(-K*(4.0*(pow(Restar,0.15))-5.0));
	rabrRe = (log((Zt-d0m)/z0h)-Psih)/(ustar*K);
	return(rabrRe);
}


double ra_funcbare(double Zt, double Zref, double d0m, double z0m,double Psih,double Psir, double ustar,double K)
{
	double rabr;
	rabr = (log(((Zt-d0m)/(Zref-d0m))-Psih+Psir))/(ustar*K);
	return(rabr);
}

/*structure version of monobf function */
void monobfs(meteo *met,tkstruct *tk,fluxstruct *Flux,refhts *Z)
{
	double termH,termLE,totterm;
	termH = (Flux->Htotal)/((met->cp)*(tk->air));
	termLE = (0.61*(Flux->LEtotal))/(met->lambdav);
	totterm = termH+termLE;
	Z->L = -(met->rhoair)*((met->ustar)*(met->ustar)*(met->ustar))/(VONKARMAN*GRAV*totterm);
}

double monobf(double ustar, double rho, double Cp, double K, double H, double LE, double tcair)
{
	double Gravity=9.8;
	double tkair, lambdav,term, L;
	/*tcair is air temperature in Celsius*/
	tkair = tcair+273.15;
	lambdav = 2501300.0-2366.0*tcair;  
	term = (H/(Cp*tkair))+(0.61*LE/lambdav);
	L = -rho*(ustar*ustar*ustar)/(K*Gravity*term);
	return(L);
}

/*sauer formula for soil resistance, structure form*/
void sauers(tkstruct *tk, meteo *met, resistor *resist)
{
	double tkdiff,b;
	tkdiff = (tk->soil)-(tk->air);
	if(tkdiff > 0.0) {
	  b = 0.0025*pow(tkdiff,(1.0/3.0));
	} else {
	  b = 0.004;
	}
	resist->soil = 1.0/(b+(0.024*(met->usoil)));
}

/*sauer formula for soil resistance */
/* tk is 4 element vector of air, soil, canopy, composite kelvin temperatures */
double sauer(double tk[],double slope,double usoil)
{
	double tkdiff,b,rsoil;
	tkdiff = tk[1]-tk[0];
	if(tkdiff > 0.0) {
	  b = 0.0025*pow(tkdiff,(1.0/3.0));
	} else {
	  b = 0.004;
	}
	rsoil = 1.0/(b+(slope*usoil));
	return(rsoil);
}

/*compute integrated Businger-Dyer Stability Functions */
void xandpsi(refhts *Z,Cover *vegcover)
{
	Z->zeta_u = ((Z->u)-(Z->d0))/(Z->L);
	Z->zeta_tt = ((Z->t)-(Z->d0))/(Z->L);
	/*  if(vegcover->frac > 0.05) { */
	Z->zeta_ref = ((Z->ref)-(Z->d0))/(Z->L);
	/*} else { */
	/*    Z->zeta_ref = ((Z->refbare)-(Z->d0bare))/(Z->L); */
	/*  } */
	/*  printf("zeta_u: %f zeta_t: %f\n",Z->zeta_u,Z->zeta_tt);*/
	Z->Xref = pow((1.0-16.0*Z->zeta_ref),0.25);
	Z->X    = pow((1.0-16.0*Z->zeta_u),0.25);
	Z->Xt   = pow((1.0-16.0*Z->zeta_tt),0.25);
	#if 0
	printf("in xandpsi: windfuncs.h\n");
	printf("Xref: %f X: %f Xt: %f\n",Z->Xref,Z->X,Z->Xt);
	#endif
	Z->Psim    = psim_func(Z->X);
	Z->Psimref = psim_func(Z->Xref);
	Z->Psih    = psih_func(Z->Xt);
	Z->Psihref = psih_func(Z->Xref);
} /*end of xandpsi */

/*compute Businger-Dyer Stability Functions */
void phiandpsi(refhts *Z,Cover *vegcover)
{
	Z->zeta_u = ((Z->u)-(Z->d0))/(Z->L);
	Z->zeta_tt = ((Z->t)-(Z->d0))/(Z->L);
	if(vegcover->frac > 0.05) {
	  Z->zeta_ref = ((Z->ref)-(Z->d0))/(Z->L);
	} else {
	  Z->zeta_ref = ((Z->refbare)-(Z->d0bare))/(Z->L);
	}
	Z->phim = phim_dyer(Z->zeta_u,Z->L);
	Z->phimref = phim_dyer(Z->zeta_ref,Z->L);
	Z->phih = phih_dyer(Z->zeta_tt,Z->L);
	Z->phihref = phih_dyer(Z->zeta_ref,Z->L);
	printf("in phiandpsi: windfuncs.h\n");
	printf("phim: %f phimref: %f phih: %f phihref: %f\n",Z->phim,Z->phimref,Z->phih,Z->phihref); 
	/*double psim_func(double x)*/
	Z->Psim = psim_func(Z->phim);
	Z->Psimref = psim_func(Z->phimref);
	/*double psih_func(double x)*/
	Z->Psih = psih_func(Z->phih);
	Z->Psihref = psih_func(Z->phihref);
	#if 0
	printf("Psi functions\n");
	printf("%f %f %f %f\n",Z->Psim,Z->Psimref,Z->Psih,Z->Psihref);
	#endif
}

/*zero out stability functions for stable conditions */
void stabphipsi(refhts *Z)
{
	/*  printf("zeroing Psi parms in stabphipsi: windfuncs.h\n");*/
	Z->Psim    = 0.0;
	Z->Psimref = 0.0;
	Z->Psih    = 0.0;
	Z->Psihref = 0.0;
}

/*compute wind related parameters for canopy covered surface */
void getwind(meteo *met,refhts *Z,Cover *vegcover,resistor *resist)
{
	double decayparm;
	met->ustar = ustar_func(VONKARMAN,met->windspeed,Z->u,Z->d0,Z->z0,Z->Psim);
	#if 0
	  resist->air = ra_func(Z->t,Z->d0,Z->z0,Z->Psih, met->ustar,VONKARMAN);
	#endif
	/* try original norman 95 formulation */
	#if 0
	ra_func_orig(double Zt, double d0m, double z0m, double Psih, double U, double K, double Psim, double Zu)
	#endif
	#if 1
	/*  printf("met.windspeed: %f\n",met->windspeed);*/
	/*  xandpsi(&Z,&vegcover);*/
	/* update stability parms */
	Z->zeta_u = ((Z->u)-(Z->d0))/(Z->L);
	Z->zeta_tt = ((Z->t)-(Z->d0))/(Z->L);
	if(vegcover->frac > 0.05) {
		Z->zeta_ref = ((Z->ref)-(Z->d0))/(Z->L);
	} else {
		Z->zeta_ref = ((Z->refbare)-(Z->d0bare))/(Z->L);
	}
	/*  printf("zeta_u: %f zeta_t: %f\n",Z->zeta_u,Z->zeta_tt);*/
	Z->Xref = pow((1.0-16.0*Z->zeta_ref),0.25);
	Z->X    = pow((1.0-16.0*Z->zeta_u),0.25);
	Z->Xt   = pow((1.0-16.0*Z->zeta_tt),0.25);
	/*  printf("X: %f Xref: %f Xt: %f\n",Z->X,Z->Xref,Z->Xt);*/
	Z->Psim    = psim_func(Z->X);
	Z->Psimref = psim_func(Z->Xref);
	Z->Psih    = psih_func(Z->Xt);
	Z->Psihref = psih_func(Z->Xref);
	/*  printf("Psim is now: %f\n",Z->Psim);*/
	resist->air = ra_func_orig(Z->t,Z->d0,Z->z0,Z->Psih,met->windspeed,VONKARMAN,Z->Psim,Z->u);
	#endif
	#if 0
	resist->air = ra_func_orig(Z->t,Z->d0,Z->z0,Z->Psih, met->ustar,VONKARMAN,Z->Psim,Z->u);
	#endif
	met->ucanopy = ucpy_func(met->windspeed,vegcover->canopyheight,Z->d0,Z->z0,VONKARMAN,met->ustar);			 
	/*  printf("ucanopy: %f\n",met->ucanopy);*/
	decayparm =  wind_decay(vegcover->LAI,vegcover->clumpfactor, vegcover->canopyheight, vegcover->leafwidth);
	/* printf("decayparm is: %f\n",decayparm);*/
	met->usoil = usoil_func(met->ucanopy,decayparm, vegcover->canopyheight,Z->d0,Z->z0);
	/* printf("usoil is: %f\n",met->usoil);*/
}
/*end of getwind function */

/*compute wind related parameters for bare soil surface */
void getwindbare(meteo *met,refhts *Z,resistor *resist, tkstruct *tk)
{
	met->ustar = ustar_func(VONKARMAN,met->windspeed,Z->u,Z->d0,Z->z0,Z->Psim);
	resist->air = ra_funcbareRe(Z->t, Z->refbare, Z->d0,Z->z0,Z->Psih,Z->Psihref,met->ustar,VONKARMAN,met->rhoair,tk->air,tk->composite);
	#if 0
	  resist->air = ra_func(Z->t,Z->d0,Z->z0,Z->Psih, met->ustar,VONKARMAN);
	#endif
	met->usoil = usoilbare_func(Z->refbare,Z->u,Z->d0bare,Z->z0,Z->Psimref,Z->Psim,met->windspeed);
} /* end of getwindbare function */

