#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tseb_eta.h"
/*by Andrew French */



/*mostly (!!some exceptions...)vegetation canopy related functions*/
/*return component temperature given the other component  */
/* temperature, fraction */
/* represented by the other component, and composite temperature */

double clump_factor(double clumpnadir,double viewangrad,double canopy_ht,double canopy_wdth)
{
	/* need to check this function,, is view angle to be in radians or degrees?? */
	/* Yann: Radians it is */
	double p,D;
	D = canopy_ht/canopy_wdth;
	p = 3.80-0.46*D;
	if(p < 1.0) p = 1.0;
	else if(p > 3.34) p= 3.34;
	return(clumpnadir/(clumpnadir+(1.0-clumpnadir)*exp(-2.2*pow(viewangrad,p))));
}

/*extract canopy temperature from soil and composite temperatures */
/* N.B.-- both this function and component_tempk_soil */
/* need to be re-examined for assignment of canopy temperature */
/* under heavy vegetation. When heavy vegetation exists, very little */
/* is known about soil temperature; furthermore the radiometric */
/* temperature should be the canopy temperature, rather than  */
/* the air temperature */

void component_tempk_canopy(double fracveg,tkstruct *tkstrucptr)
{
	double numer;
      	if(fracveg <0.8 && fracveg >= 0.0 ) {
		numer = pow(tkstrucptr->composite,4.0)-((1.0-fracveg)*pow(tkstrucptr->soil,4.0));
		tkstrucptr->canopy = pow(numer/fracveg,0.25);
	} else tkstrucptr->canopy = tkstrucptr->air;
}

/*extract soil temperature from canopy and composite temperatures */
void component_tempk_soil(double fracveg,tkstruct *tkstrucptr)
{
	double numer;
	if(fracveg <0.8 && fracveg >= 0.0 ) {
		numer = pow(tkstrucptr->composite,4.0)-(fracveg*pow(tkstrucptr->canopy,4.0));
		tkstrucptr->soil = pow(numer/(1.0-fracveg),0.25);
	} else tkstrucptr->soil = tkstrucptr->air;
}

double component_tempk(double frac1,double tk1,double composite_tk)
{
	/*since algorithm is unstable form fractional cover greater than 80%, */
	/* need to have check for this condition */
	float numer,tk2;
	if(frac1 < 0.8 && frac1 >= 0.0) {
		numer = pow(composite_tk,4.0)-(frac1*pow(tk1,4.0));
		tk2 = pow(numer/(1.0-frac1),0.25);
	} else tk2 = composite_tk;
	return(tk2);
}

double reflhorfunc(double absorptivity)
{
	double rho;
	rho = (1.0-sqrt(absorptivity))/(1.0+sqrt(absorptivity));
	return(rho);
}

/*beam reflection coefficient for deep canopy */
/* where leaves are not horizontal*/
double rhonothorfunc(double rhohor, double Kbe)
{
	double rhonothor;
	rhonothor = (2.0*Kbe/(Kbe+1.0))*rhohor;
	return(rhonothor);
}

/*reflection coefficient for sparse canopy*/
/* beam or diffuse irradiance. K is exinction coefficient*/
/* Omega is clumping factor, LAI is leaf area index*/
double rhocpyfunc(double rhonothor, double rhosoil,double absorptivity,double K,double LAI,double clump_factor)
{
	double expterm,fracterm,rho_sparse;
	expterm = exp(-2.0*sqrt(absorptivity)*K*LAI*clump_factor);
	fracterm = (rhonothor-rhosoil)/(rhonothor*rhosoil-1.0);
	rho_sparse = (rhonothor+fracterm*expterm)/(1.0+rhonothor*fracterm*expterm);
	return(rho_sparse);
}

/* Extinction coefficient for black leaves*/
/* ellipsoidal distribution, beam radiation*/
/* Psi is solar zenith angle in degrees*/
/* x is ratio of horizontal to vertical projected elements*/
double kbefunc(double x, double Psirad)
{
	double tPsirad,numer,denom,K;
	tPsirad = tan(Psirad);
	numer = sqrt((x*x)+(tPsirad*tPsirad));
	denom = x+1.774*pow(x+1.182,-0.733);
	/*  denom = pow((x+1.774*(x+1.182)),-0.733);*/
	K = numer/denom;
	return(K);
}

double frac_cover_choud(double ndvi_min,double ndvi_max,double ndvi_obs,double choud_p)
{
	double nc,fcover;
	double ndviscale= 0.999;
	/*p ranges from 0.5 for dark soil to 0.7 for bright soil*/
	/*p=0.6 is reasonable for most location */
	/* reset ndvi_obs if it exceeds threshold values */
	if(ndvi_obs > ndviscale*ndvi_max) ndvi_obs = ndviscale*ndvi_max;
	else if(ndvi_obs < ndvi_min) ndvi_obs = ndvi_min;
	nc = (ndvi_max-ndvi_obs)/(ndvi_max-ndvi_min);
	fcover = 1.0-pow(nc,choud_p);
	return(fcover);
}


double LAI_choudfunc(double frac_cover,double Beta)
{
	/*returns leaf area index based on Choudhury 1987*/
	/* also Beta from Choudhury 1994*/
	/*Beta is typically 0.67*/
	/* return -2 if frac_cover 1.0 or greater, or */
	/* -1 if frac_cover less than 0.0 */
	double LAI;
	if (frac_cover < 0.0) LAI = -1;
	else if (frac_cover >= 1.0) LAI = -2;
	else LAI = -(log(1.0-frac_cover))/Beta;
	return(LAI);
}

double LAI_carlson(double NDVI,double NDVImin,double NDVImax)
{
	double tNDVI,LAI;
	tNDVI = (NDVI-NDVImin)/(NDVImax-NDVImin);
	LAI = tNDVI*tNDVI;
	return(LAI);
}

/*compute canopy heat fluxes */
void ehfluxes(double green, double PT, meteo *met,fluxstruct *Flux)
{
	double lval;
	lval = (met->desat_dtk)/((met->desat_dtk)+(met->gamma));
	Flux->LEcanopy = green*PT*lval*(Flux->Rncanopy);
	Flux->Hcanopy = (Flux->Rncanopy)-(Flux->LEcanopy);
	if(isnan(Flux->Hcanopy)) {
		printf("lval is: %f and Rncanopy: %f\n",lval,Flux->Rncanopy);
		exit(1);
	}
	#if 0
	printf("In ehfluxes:canopy.h:\n");
	printf("LEcanopy: %f Hcanopy: %f\n",Flux->LEcanopy,Flux->Hcanopy);
	#endif
}

/*long wave radiation, structure version*/
void getrls(meteo *met, tkstruct *tk,soilabsemiss *soilabs_ems,leafabsemiss *leafabs_ems,LngWave *RL)
{
	double stefanboltz=5.67e-8;
	double e_atm;
	/*Brutsaert formula for atmosperic emissivity */
	#if 0
	from idl program
	Rncanopy = (1.0-tautherm)*(rlsky+rlsoil-2.0*rlcanopy)+(1.0-tausolar)*(1.0-albedocanopy)*Rs
	Rnsoil = (tautherm*rlsky)+((1.0-tautherm)*rlcanopy)-rlsoil + tausolar*(1.0-albedosoil)*Rs
	#endif
	e_atm   = 1.24*pow((met->ea/tk->air),(1.0/7.0));
	RL->soil   = soilabs_ems->emisstir*stefanboltz*pow(tk->soil,4);
	RL->canopy = leafabs_ems->emisstir*stefanboltz*pow(tk->canopy,4);
	RL->air    = e_atm*stefanboltz*pow(tk->air,4);
	#if 0
	printf("tkair: %f\n",tk->air);
	printf("tksoil: %f\n",tk->soil);
	printf("tkcanopy: %f\n",tk->canopy);
	printf("e_atm: %f\n",e_atm);
	printf("Rsoil: %f\n",RL->soil);
	printf("Rcanopy: %f\n",RL->canopy);
	printf("Rair: %f\n",RL->air);
	#endif
}

/*check for computed condensation and return 1 or 2 if it occurs*/
/* 1 means condensation on soil, 2 means condensation on canopy */
/*fix H and LE values so condensation not allowed */
int nocondense(fluxstruct *Flux, tkstruct *tk,resistor *resist, meteo *met,Cover *vegcover)
{
	short int condenseflag=0;
// 	double Bos=10.0;
	if(Flux->LEsoil  < 0.0) {
		#if 0
		printf("LE soil less than zero! %f\n",Flux->LEsoil);
		printf("tksoil: %f tkcanopy: %f tkcomposite: %f tkair: %f\n",tk->soil,tk->canopy,tk->composite,tk->air);
		printf("rsoil: %f rair: %f\n",resist->soil,resist->air);
		printf("Hsoil: %f Hcanopy: %f G: %f Rnsoil: %f Rncanopy: %f Rntotal: %f\n",Flux->Hsoil,Flux->Hcanopy,Flux->G,Flux->Rnsoil,Flux->Rncanopy,Flux->Rntotal);
		exit(1);
		#endif
		#if 0
		Flux->LEsoil = (Flux->Rnsoil-(Flux->G))/(1.0+Bos);
		#endif
		Flux->LEsoil = 0.0;
		Flux->Hsoil = Flux->Rnsoil-(Flux->G)-(Flux->LEsoil);
		/*compute soil surface temperature given Hsoil */
		tempresist_soil(Flux,tk,resist,met);
		/*get canopy temperature */
		component_tempk_canopy(vegcover->frac,tk);
		/*get Hcanopy from canopy temperature */
		hfluxresist_canopy(tk,resist,Flux,met);
		/*get LEcanopy as a residual */
		Flux->LEcanopy = (Flux->Rncanopy)-(Flux->Hcanopy);
		condenseflag = 1;
	} /*end of if(Flux->LEsoil < 0.0 */
	if(Flux->Hcanopy > Flux->Rncanopy) {
		Flux->LEsoil = 0.0;
		Flux->LEcanopy = 0.0;
		Flux->Hcanopy = Flux->Rncanopy;
		printf("in trap to catch negative LEcan.\n");
		printf("Hcanopy: %f\n",Flux->Hcanopy);
		/*recompute canopy temperature */
		tempresist_can(Flux,tk,resist,met);
		/*recompute soil temperature */
		tempresist_soil(Flux,tk,resist,met);
		/* revise Hsoil */
		hfluxresist_soil(tk,resist,Flux,met);
		/* get soil flux G as residual */
		Flux->G = (Flux->Rnsoil)-(Flux->Hsoil);
		condenseflag = 2;
	} /*end of if(Flux->Hcanopy > Flux-> Rncanopy */
	return(EXIT_SUCCESS);
} /*end of nocondense */

/*compute fluxes for one layer condition */
/* this function already checks for condensation, so dont do it later*/
void onelayer(fluxstruct *Flux,tkstruct *tk, double cg, meteo *met,Albeds *albedvegcan, soilabsemiss *soilabs_ems, leafabsemiss *leafabs_ems, resistor *resist, LngWave *RL)
{
	/*  double sigma= 5.67e-8;*/
	double Bos=10.0;
// 	double totsoilresist;
	/*sauer equation with Kondo Ishida value for b*/
	sauers(tk, met, resist);
	albedvegcan->soil = 0.5*((1.0-soilabs_ems->vis)+(1.0-soilabs_ems->nir));
	getrls(met,tk,soilabs_ems,leafabs_ems,RL);
	Flux->Rnsoil = (RL->air)-(RL->soil)+(1.0-(albedvegcan->soil))*(met->Rsolar);
	Flux->Rncanopy = 0.0;
	Flux->Rntotal = Flux->Rnsoil;
	Flux->Hcanopy = 0.0;
	Flux->LEcanopy = 0.0;
	Flux->G = cg*(Flux->Rnsoil);
	/* printf("soil albedo: %f Rsolar %f\n",albedvegcan->soil,met->Rsolar);*/
	/*  printf("Net radiation: %f\n",Flux->Rntotal);*/
	/*  totsoilresist = (resist->soil)+(resist->air);*/
	/*Compute Hsoil */
	hfluxresist_soil(tk,resist,Flux,met);
	/*Compute LEsoil as residual */
	Flux->LEsoil = Flux->Rnsoil-(Flux->G)-(Flux->Hsoil);  
	/*Dont allow condensation on soil surface */
	if(Flux->LEsoil < 0.0) {
		Flux->LEsoil = ((Flux->Rnsoil)-(Flux->G))/(1.0+Bos);
		Flux->Hsoil = (Flux->Rnsoil)-(Flux->G)-(Flux->LEsoil);
		tempresist_soil(Flux,tk,resist,met);
	}
}


/*twolayer computes turbulent fluxes for a two layer condition */
/* Hsoil, Hcanopy, LEsoil, LEcanopy */

void twolayer(tkstruct *tk,fluxstruct *Flux,meteo *met,resistor *resist,Cover *vegcover)
{
	/*compute canopy fluxes */
	/*void ehfluxes(double green, double PT, meteo *met,fluxstruct *Flux)*/
	ehfluxes(vegcover->green,vegcover->PT,met,Flux);
	/*compute canopy temperature */
	/*find temperature of canopy from LEcan*/
	/* tempresist_can(Flux,tk,resist,met);*/
	/*now find soil temperature */
	/*compute temperature based on fractional averaging */
	/*tk.soil is soil temperature */
	/* printf("in two layer function tk soil: %f\n",tk->soil);*/
	/* N.B.!!!!!!   */
	/* 24 November 2004 */
	/* temporary configuration set tk.soil to tksoilmax value */
	/* this should cause LEsoil to go to zero */
	#if 0
	printf("in canopy.h:Rnsoil: %f G: %f\n",Flux->Rnsoil,Flux->G);
	#endif
	/* try not revising tk->soil */
	/* tk->soil = pow(((Flux->Rntotal)-(Flux->G))/(0.95*STEFANBOLTZ),0.25);*/
	#if 0
	printf("in twolayer function, updating tk.soil!\n"); 
	printf("tksoil: %f\n",tk->soil);
	printf("tkcanopy: %f\n",tk->canopy);
	#endif
	/*pow(((Flux->Rnsoil-Flux->G)/(0.95*STEFANBOLTZ)),0.25);*/
	/* component_tempk_soil(vegcover->frac,tk);*/
	/*use sauer equation */
	sauers(tk,met,resist);
	/*get Hsoil */
	/* temporary reset to check soil temperature */
	if(tk->soil < tk->air) {
		tk->soil = tk->air;
	}
	/* tk->soil = tk->composite;*/
	hfluxresist_soil(tk,resist,Flux,met);
	/* printf("in twolayer functions, Hsoil: %f\n",Flux->Hsoil);*/
	/*get LEsoil as residual */
	Flux->LEsoil = (Flux->Rnsoil)-(Flux->G)-(Flux->Hsoil);
	Flux->Htotal = Flux->Hsoil + Flux->Hcanopy;
	Flux->LEtotal = Flux->LEsoil + Flux->LEcanopy;
} /*end of twolayer function */

/*compute canopy reflectivities in vis and nir for direct and diffuse light */
/* also compute canopy transmissivities in vis and nir for direct and diffuse */

void canopyrho(meteo *met,Cover *vegcover,radweights *radwts, leafabsemiss *leafabs_ems,soilabsemiss *soilabs_ems,CanopyLight *cpylight)
{
	double rhohorvis,rhohornir;
	double rhostarvisdir,rhostarnirdir;
	double rhostarvisdif,rhostarnirdif;
// 	double rhovisdif,rhonirdif;
// 	double rhovisdir,rhonirdir;
	double rhosoilvis,rhosoilnir;
	#if 0
	printf("leafangparm: %f\n",vegcover->leafangparm);
	printf("solzenangrad: %f\n",met->solzenangrad);
	#endif
	radwts->Kbe =  kbefunc(vegcover->leafangparm,met->solzenangrad);
	rhohorvis = reflhorfunc(leafabs_ems->vis);
	rhohornir = reflhorfunc(leafabs_ems->nir);
	rhosoilvis = 1.0-(soilabs_ems->vis);
	rhosoilnir = 1.0-(soilabs_ems->nir);
	/*  printf("Kbe: %f\n",radwts->Kbe);*/
	rhostarvisdir = rhonothorfunc(rhohorvis,radwts->Kbe);
	rhostarnirdir = rhonothorfunc(rhohornir,radwts->Kbe);
	rhostarvisdif = rhonothorfunc(rhohorvis,radwts->Kd);
	rhostarnirdif = rhonothorfunc(rhohornir,radwts->Kd);
	#if 0
	printf("rhostarvisdir %f\n",rhostarvisdir);
	printf("rhostarnirdir %f\n",rhostarnirdir);
	printf("rhostarvisdif %f\n",rhostarvisdif);
	printf("rhostarnirdif %f\n",rhostarnirdif);
	exit(1);
	#endif
	/*get canopy reflectivities in visible and near infrared */
	/* for direct and diffuse radiation */
	cpylight->rhovisdir = rhocpyfunc(rhostarvisdir,rhosoilvis,leafabs_ems->vis,radwts->Kbe,vegcover->LAI,vegcover->clumpfactor);
	cpylight->rhonirdir = rhocpyfunc(rhostarnirdir,rhosoilnir,leafabs_ems->nir,radwts->Kbe,vegcover->LAI,vegcover->clumpfactor);
	cpylight->rhovisdif = rhocpyfunc(rhostarvisdif,rhosoilvis,leafabs_ems->vis,radwts->Kd,vegcover->LAI,vegcover->clumpfactor);
	cpylight->rhonirdif = rhocpyfunc(rhostarnirdif,rhosoilnir,leafabs_ems->nir,radwts->Kd,vegcover->LAI,vegcover->clumpfactor);
	/*get canopy transmissivities in vis and nir for direct and diffuse light */
	cpylight->tauvisdir = canopytran(leafabs_ems->vis,radwts->Kbe,vegcover->LAI, vegcover->clumpfactor,rhostarvisdir,rhosoilvis);
	cpylight->taunirdir = canopytran(leafabs_ems->nir,radwts->Kbe,vegcover->LAI,vegcover->clumpfactor,rhostarnirdir,rhosoilnir);
	cpylight->tauvisdif = canopytran(leafabs_ems->vis,radwts->Kd,vegcover->LAI,vegcover->clumpfactor,rhostarvisdif,rhosoilvis);
	cpylight->taunirdif = canopytran(leafabs_ems->nir,radwts->Kd,vegcover->LAI,vegcover->clumpfactor,rhostarnirdif,rhosoilnir);
	#if 0
	printf("tauvisdir: %f\n",cpylight->tauvisdir);
	printf("taunirdir: %f\n",cpylight->taunirdir);
	printf("tauvisdif: %f\n",cpylight->tauvisdif);
	printf("taunirdif: %f\n",cpylight->taunirdif);
	exit(1);
	#endif
	/*thermal infrared transmissivity treated by simple extinction */
	/* printf("LAI in tautir function: %f\n",vegcover->LAI);*/
	cpylight->tautir = exp(-(leafabs_ems->extinct)*(vegcover->LAI)*(vegcover->clumpfactor));
	/* printf("tautir: %f\n",cpylight->tautir);*/
	/*finished with canopy transmissivity */
} /* end of canopyrho */


/* transmissivity of canopy from Campbell and Norman 98 */
double canopytran(double absorptivity,double Kextinct, double LAI, double clumpfactor, double rhonothor,double rhosoil)
{
	double subtrm,numer,denom,tauval;
	subtrm = sqrt(absorptivity)*Kextinct*LAI*clumpfactor;
	numer = (rhonothor*rhonothor-1.0)*exp(-subtrm);
	denom = (rhonothor*rhosoil-1.0)+rhonothor*(rhonothor-rhosoil)*exp(-2.0*subtrm);
	tauval = numer/denom;
	return(tauval);
}

/*compute soil albedo */
void rhosoil2albedo(soilabsemiss *soilabs_ems,Albeds *albedvegcan)
{
	double rhosoilvis,rhosoilnir; 
	rhosoilvis = 1.0-(soilabs_ems->vis);
	rhosoilnir = 1.0-(soilabs_ems->nir);
	/* set Soil Albedo here */
	/* in future, get it from imagery */ 
	albedvegcan->soil = 0.5*(rhosoilvis+rhosoilnir);
}

/*compute canopy albedo and transmissivity as a weighted sum */
void rhocpy2albedo(CanopyLight *cpylight,Albeds *albedvegcan,radweights *radwts)
{
	double albed_dir,albed_dif,sumofwts;
	double trans_dir,trans_dif;
	albed_dir = ((radwts->visdir)*(cpylight->rhovisdir))+((radwts->nirdir)*(cpylight->rhonirdir));
	albed_dif = ((radwts->visdif)*(cpylight->rhovisdif))+((radwts->nirdif)*(cpylight->rhonirdif));
	trans_dir = ((radwts->visdir)*(cpylight->tauvisdir))+((radwts->nirdir)*(cpylight->taunirdir));
	trans_dif = ((radwts->visdif)*(cpylight->tauvisdif))+((radwts->nirdif)*(cpylight->taunirdif));
	sumofwts = (radwts->visdir)+(radwts->nirdir)+(radwts->visdif)+(radwts->nirdif);
	#if 0 
	printf("albed_dir: %f albed_dif: %f\n",albed_dir,albed_dif);
	printf("trans_dir: %f trans_dif: %f\n",trans_dir,trans_dif);
	#endif
	/*now write albedo of canopy */
	albedvegcan->canopy = (albed_dir+albed_dif)/sumofwts;
	/* and write vnir transmissivity of canopy*/
	cpylight->tausolar = (trans_dir+trans_dif)/sumofwts;
}

void transcpy(double rhostar,double rhosoil,double alpha,double K,double FOmega,double taubt)
{
	double subtrm,numer,denom;
	#if 0
	procedure computes transmitted beam radiation through canopy
	including scattered and direct radiation
	rhostar is canopy directional-hemispherical reflectance for
	    beam radiation incident at angle psi for canopy of infinite
	    leaf area index
	rhosoil soil reflectivity
	alpha is leaf absorbtivity
	K is either :Kbe is black leaf canopy extinction coefficient for ellipsoidal
	leaf angle distribution, beam radiation
	or K is Kd: extinction coefficient of black leaf canopy for diffuse 
	radiation
	FOmega is leaf area index above some canopy height multiplied by clumping factor
	#endif
	subtrm = sqrt(alpha)*K*FOmega;
	numer = (rhostar*rhostar-1.0)*exp(-subtrm);
	denom = (rhostar*rhosoil-1.0)+rhostar*(rhostar-rhosoil)*exp(-2.0*subtrm);
	taubt = numer/denom;
}

/**--------------------------------------------------------------------------**/
/**WATER FUNCTIONS ************************************************************/
/**--------------------------------------------------------------------------**/

/* psychrometric constant from FAO56 method */
double get_psychroFAO(double preskPa) {
	double latenthtvap = 2.45;
	double cp= 1.013e-3;
	double molwtratio = 0.622;
	double gamma;
	gamma = (cp*preskPa)/(molwtratio*latenthtvap);
	return(gamma);
}

/* convert elevation above sea level (m) to atmospheric pressure (kPa) */
double z2P_fao(double z) {
	double P;
	P = 101.3*(pow(((293.0-0.0065*z)/293.0),5.26));
	return(P);
}

/* vapor pressure from dry bulb and wet bulb measurement */
double psychro2ea(double preskPa,double tcdrybulb, double tcwetbulb,double psychrocoef)
{
	/* embed Richards estimate for esat */
	/*pschrocoef is 0.000662 for Asmann, 0.0008 for natural ventilated */
	/* and 0.0012 non-ventilated indoor observation */
	/* if set to negative number, default to Asmann value */
	double eakPa,esatkPa;
	double psychroconst;
	double c,a1,a2,a3,a4,tr;
	if(psychrocoef < 0.0) {
		psychrocoef = 0.000662;
	}
	psychroconst = psychrocoef*preskPa;
	/* rewritten for kPa */
	c  = 101.325;
	a1 =   13.3185;
	a2 =   -1.9760;
	a3 =   -0.6445;
	a4 =   -0.1299;
	tr = 1.0-(373.15/(tcwetbulb+273.15));
	esatkPa = c*exp(tr*(a1+tr*(a2+tr*(a3+tr*a4))));
	eakPa = esatkPa - psychroconst*(tcdrybulb-tcwetbulb);
	return(eakPa);
}


/* Richards' approximation of saturation vapor pressure */ 
/*   in mbar and slope of temperature                           */
/*   vapor pressure curve                               */
/* work with pointer */
void richesat_ptr(double tk, double *esatmb_p, double *eslope_p)
{
	double c,a1,a2,a3,a4,tr,lesat;
	c  = 1013.25;
	a1 =   13.3185;
	a2 =   -1.9760;
	a3 =   -0.6445;
	a4 =   -0.1299;
      
	tr = 1.0-(373.15/tk);
	lesat = c*exp(tr*(a1+tr*(a2+tr*(a3+tr*a4))));
	*eslope_p = (373.15*lesat/(tk*tk))*(a1+tr*(2*a2+tr*(3*a3+tr*(4*a4))));
	*esatmb_p = lesat;
}

/* Richard's approximation of saturation vapor pressure */ 
/*   in mbar                           */
/* work with value , only return saturation vapor pressure in mbar */
double richesat_val(double tk)
{
	double c,a1,a2,a3,a4,tr,lesat;
	c  = 1013.25;
	a1 =   13.3185;
	a2 =   -1.9760;
	a3 =   -0.6445;
	a4 =   -0.1299;
      
	tr = 1.0-(373.15/tk);
	lesat = c*exp(tr*(a1+tr*(a2+tr*(a3+tr*a4))));
	return(lesat);
}

/*vapor pressure to mixing ratio */
double e2r(double embar, double Pmbar)
{
	double r;
	r = 0.62197/((Pmbar/embar)-1.0);
	return(r);
}

/*mixing ratio to vapor pressure*/
double r2e(double r, double Pmbar)
{
	double e;
	e = (Pmbar*r)/(0.62197+r);
	return(e);
}


/* Density of water vapor from vapor pressure in mbar */
/* density in g/m^3 */
double e_mb2rhoden(double e_mbar, double tkair)
{
	double lrho;
	lrho =  (MolWtH2O/MolWtDryAir)*e_mbar/(Rd*1e-5*tkair);
	return(lrho);
}

/*Density of dry air component from vapor pressure in mbar*/
/* and total pressure in mbar, and ambient temperature in Kelvin*/
/* Rd used here is mbar m^3/(Kelvin-g) */
double rhodry(double e_mbar, double p_mbar, double tkelvin)
{
	double rhodry;
	rhodry = (p_mbar-e_mbar)/(2.8704e-3*tkelvin);
	return(rhodry);
}

/* Convert relative humidity to vapor density */
/* Relative humidty as value between 0 and 1 */
/* Vapor density in g/m^3 */
double rh2vapden(double rh, double press_mbar, double tkelvin)
{
	double esat,vapden;
	esat = richesat_val(tkelvin);
	vapden = rh*esat*MolWtH2O/(tkelvin*Rd*0.01);
	return(vapden);
}

/*Convert mixing ratio to vapor density */
/* Output in g/m^3 */
double mixr2vapden(double mixr,double tkelvin, double press_mbar)
{
	double vapden;
	double epsilon;
	epsilon = Rd/Rv;
	vapden = ((1.0+mixr)/(1.0+(mixr/epsilon)))*(press_mbar/(Rd*tkelvin));
	return(vapden);
}

/*Convert mixing ratio to specific humidity */
double mixr2spechum(double mixr)
{
	double spechum;
	spechum = mixr/(mixr+1.0);
	return(spechum);
}

/*Convert specific humidity to mixing ratio*/
double spechum2mixr(double spechum)
{
	double mixr;
	mixr = spechum/(1.0-spechum);
	return(mixr);
}

/* Virtual temperature from ambient temperature and mixing ratio*/
/* Input Kelvin temperature and mixing ratio*/

double ta_mixr2tvirtual(double tk, double wmix)
{
	double tkv;
	double mwratio;
	mwratio = MolWtH2O/MolWtDryAir;
	tkv = tk*(mwratio+wmix)/(mwratio*(1.0+wmix));
	return(tkv);
}

/*Virtual temperature from ambient temperature and specific humidity*/
/*Input Kelvin temperature and specific humidity*/
double ta_spechum2tvirtual(double tk, double spechum)
{
	double tkv;
	tkv = (1.0+0.61*spechum)*tk;
	return(tkv);
}


/* Convert mbar pressure to standard atmosphere */
double mbar2stdatmo(double p_mbar)
{
	double stdatmo;
	stdatmo = p_mbar/1013.25;
	return(stdatmo);
}


/* Change in altitude from hypsometric equation */
/* pres2 less than pres1 */
/* tbar in Kelvin, average temperature of interval */
double hypso_del_alt(double tbar,double pres1,double pres2)
{
	double dalt;
	dalt =-((Rd*1000.0*tbar)/GRAV)*log(pres2/pres1);
	return(dalt);
}

/* Celsius to Kelvin */
double c2k(double tcelsius)
{
	double tkelvin;
	tkelvin = tcelsius+273.15;
	return(tkelvin);
}

/*Kelvin to Celsius */
double k2c(double tkelvin)
{
	double tcelsius;
	tcelsius = tkelvin-273.15;
	return(tcelsius);
}

double tk2rad(double tk,double lambda,double emiss)
{
      double term,lam,rad;

      lam = lambda*1e-6;
      term = exp(c2/(lam*tk))- 1.0;
      rad = (emiss*c1/((pow(lam,5))*PI*term))*1000.0;
      return(rad);
}

double rad2tk(double rad,double lambda,double emiss)
{
      double term,lam,tk;
      lam = lambda*1e-6;
      term = ((emiss*c1)/(pow(lam,5)*PI*rad*0.001)) + 1.0;
      tk = c2/(lam*log(term));
      return(tk);
}

/*density of moist air */
double e2rhomoist(double embar,double presmbar,double tk) 
{
      double myrd,rhotot;
      myrd = 2.8704;
      rhotot = (presmbar/(myrd*tk))*(1.0-((0.378*embar)/presmbar));
      return(rhotot);
}

/*Specific heat of moist air given vapor pressure, air density, air temperature*/
double cpd2cp_airden(double e_mbar, double rhoair, double tcair)
{
      /*cpd is 1004.67 J/(kg-Kelvin) */
      /*e_mbar is vapor pressure in mbar */
      /* rhoair is air density in g/m^3 */
      /*tcair is air temperature in Celsius*/
      double cp=1004.67;
      double rhov,q,cpw;
      rhov = 0.622*e_mbar*100.0/(287.04*(tcair+273.15));
      q = rhov/rhoair;
      cpw = cp*(1.0+0.84*q);
      return(cpw);
}

/*Specific heat of moist air given specific humidity*/
double cpd2cp(double q)
{
      double cpd=1004.67;
      double cp;
      cp = cpd*(1.0+0.84*q);
      return(cp);
}

/*Latent heat of vaporization from air temperature */
double latenthtvap(double tcair)
{
      double lv;
      /*tcair is air temperature in Celsius */
      /* lv is latent heat of vaporization in J/kg */
      lv = (2.501-0.00237*tcair)*1.0e6;
      return(lv);
}

/*Psychrometric constant times pressure */
double gammafunc(double tkair,double pressmbar,double cpJKkg,double lambdav)
{
      /* specific heat of moist air in J/(Kelvin-kg) */
      /* air temperature in Kelvin, pressure in mbar */
      /* cp for dry air is 1004.67 J/(Kelvin-kg) */
      /* lambdav is latent heat of vaporization in J/kg */
      /* at 20 Celsius it is 2.45x10^6 J/kg */
      double g;
      g = cpJKkg*pressmbar/(0.622*lambdav);
      return(g);
}

/*returns reynolds number over water */
double reynolds(tkstruct *tk, meteo *met,refhts *Z)
{
      double mu0 = 1.832e-5;
      double z0mwater = 0.00035;
      double tas;
      double mu,nu,Re;

      Z->z0 = z0mwater;
      tas = 0.5*((tk->air)+(tk->composite));
      /*viscosities */
      mu = ((296.16+393.16)/(tas+393.16))*pow((tas/296.16),(3.0/2.0))*mu0;
      nu = mu/(met->rhoair);
      Re = (met->ustar)*z0mwater/nu;
      return(Re);
}

double getreynoldsnum(double tcair, double tc_composite,
		      double ustar,double z0m,double rhoair,
		      double mu0)
{
	double tas,mu,nu,re;
	tas = 0.5*(tcair+tc_composite);
	mu = ((296.16+393.16)/(tas+393.16))*pow((tas/296.16),(3.0/2.0))*mu0;
	nu = mu/rhoair;
	re = ustar*z0m/nu;
	return(re);
}

/*get Hcanopy from resistance equation */
void hfluxresist_canopy( tkstruct *tkstrucptr,resistor *resist, 
			 fluxstruct *Fluxstrucptr,meteo *met)
{
	if(resist->air <= 0.0001) {
		printf("Resistance value near zero!! Setting Hflux to -9999.9!!\n");
		Fluxstrucptr->Hcanopy = -9999.9;
	} else {
		Fluxstrucptr->Hcanopy = (((tkstrucptr->canopy)-(tkstrucptr->air))*
				  (met->rhoair)*(met->cp))/(resist->air);
	}
} /* end of hfluxresist_canopy */

/*get Hsoil from resistance equation */
void hfluxresist_soil( tkstruct *tkstrucptr,resistor *resist, 
		       fluxstruct *Fluxstrucptr,meteo *met)
{
	double totresist;
	totresist = (resist->soil)+(resist->air);
	if(totresist <= 0.0001) {
		printf("Resistance value near zero!! Setting Hflux to -9999.9!!\n");
		Fluxstrucptr->Hsoil = -9999.9;
	} else {
		Fluxstrucptr->Hsoil = (((tkstrucptr->soil)-(tkstrucptr->air))*
				  (met->rhoair)*(met->cp))/totresist;
	}
} /* end of hfluxresist_soil */



/*returns Hflux from resistance equation */
double hfluxresist(double tc_soil,double tc_air,double resist,
		   double rhoair,double cp)
{
	double Hflux;
	if(resist == 0.) {
		printf("Resistance value of zero!! Setting Hflux to -9999.9!!\n");
		Hflux = -9999.9;
	} else {
		Hflux = ((tc_soil-tc_air)*rhoair*cp)/resist;
	}
	return(Hflux);
}

/*Compute soil surface temperature given Hflux and air temperature */
void tempresist_soil(fluxstruct *Fluxstrptr,
		     tkstruct *tkstrptr,resistor *resist,
		     meteo *met)
{
	double totresist;
	totresist = resist->soil+resist->air;
	tkstrptr->soil = tkstrptr->air+((Fluxstrptr->Hsoil)*
					totresist/((met->rhoair)*(met->cp))); 
}

/*Compute canopy vegetation surface temperature given */
/* Hflux and air temperature */
void tempresist_can(fluxstruct *Fluxstrptr,
		    tkstruct *tkstrptr,resistor *resist,
		    meteo *met)
{
	#if 0
	  printf("in tempresist_can:water.h\n");
	  printf("Hcanopy: %f r_air: %f rhoair: %f cp %f\n",Fluxstrptr->Hcanopy,resist->air,met->rhoair,met->cp);
	#endif
	tkstrptr->canopy = tkstrptr->air+((Fluxstrptr->Hcanopy)*(resist->air)/((met->rhoair)*(met->cp))); 
	#if 0
	  printf("in tempresist_can/water.h: tkstrptr->canopy: %f\n",tkstrptr->canopy);
	#endif
}

double deg2rad(double degrees)
{
	double radval;
	radval = degrees*M_PI/180.0;
	return(radval);
}

double rad2deg(double radianval)
{
	double degval;
	degval = radianval*180.0/M_PI;
	return(degval);
}

/*structure version of radiatewater */

void radiatewaters(tkstruct *tk,fluxstruct *Flux, meteo *met,Albeds *albedvegcan)
{
	/*  printf("in water radiatewaters!\n");*/
	double e_atm;
	double albedowater = 0.1;
	/*Brutsaert formula for atmosperic emissivity */
	e_atm   = 1.24*pow((met->ea/tk->air),(1.0/7.0));
	Flux->Rntotal = (1.0-albedowater)*(met->Rsolar)*e_atm*STEFANBOLTZ*pow((tk->air),4.0);
	Flux->G = 0.55*(Flux->Rntotal);
	Flux->Rnsoil = Flux->Rntotal;
	Flux->Rncanopy = 0.0;
}

void radiatewater(double tk[],double Rn[],double *G,double tc_air,double evap_mbar,double R_solar)
{
	double tk_air;
	double epsilons= 0.99;
	double epsilona ;
	double albedo = 0.1;
	double stef_boltz = 5.67e-8;

	tk_air = tc_air+273.15;
	epsilona = 1.24*pow((evap_mbar/tk_air),(1.0/7.0));
	Rn[2] = (1.0-albedo)*R_solar*epsilona*stef_boltz*pow(tk_air,4.0)-epsilons*stef_boltz*pow(tk[2],4.0)-(1.0-epsilons)*epsilona*stef_boltz*pow(tk_air,4.0);
	*G = 0.55*Rn[2];
	Rn[0] = Rn[2];
	Rn[1] = 0.0;
}

/*Compute net radiation and G*/
void getRnG(CanopyLight *cpylight,Albeds *albedvegcan,LngWave *RL,meteo *met,fluxstruct *Flux,double cg, Cover *vegcover)
{
	#if 0
	printf("in RnG function.\n");
	printf("fractional cover: %f\n",vegcover->frac);
	printf("tautir: %f\n",cpylight->tautir);
	printf("RLair:%f\n",RL->air);
	printf("RLsoil: %f\n",RL->soil);
	printf("RLcanopy: %f\n",RL->canopy);
	printf("tausolar: %f\n",cpylight->tausolar);
	printf("albedo canopy: %f\n",albedvegcan->canopy);
	printf("Rsolar: %f\n",met->Rsolar);
	#endif
	#if 0
	from idl program
	Rncanopy = (1.0-tautherm)*(rlsky+rlsoil-2.0*rlcanopy)+(1.0-tausolar)* (1.0-albedocanopy)*Rs
	Rnsoil = (tautherm*rlsky)+((1.0-tautherm)*rlcanopy)-rlsoil + tausolar*(1.0-albedosoil)*Rs
	#endif
	#if 0
	/* from idl version */
	Flux->Rncanopy = (1.0-(cpylight->tautir))*
	((RL->air)+(RL->soil)-(2.0*(RL->canopy))) + (1.0-(albedvegcan->canopy))*(met->Rsolar);
	Flux->Rnsoil = ((cpylight->tautir)*(RL->air))+((1.0-(cpylight->tautir))*(RL->canopy))-(RL->soil)+((cpylight->tausolar)*(1.0-(albedvegcan->soil))*(met->Rsolar));
	#endif
	#if 1
	/* revised by FJacob and Afrench */
	Flux->Rncanopy = (1.0-(cpylight->tautir))*((RL->air)+(RL->soil)-(2.0*(RL->canopy)))+(1.0-(cpylight->tausolar))*(1.0-(albedvegcan->canopy))*(met->Rsolar);
	Flux->Rnsoil = ((cpylight->tautir)*(RL->air))+((1.0-(cpylight->tautir))*(RL->canopy))-(RL->soil)+((cpylight->tausolar)*(1.0-(albedvegcan->soil))*(1.0-(albedvegcan->canopy))*(met->Rsolar));
	#endif
	Flux->Rntotal = (Flux->Rnsoil) + (Flux->Rncanopy);
	if(isnan(Flux->Rntotal)) {
		printf("Rncanopy: %f Rnsoil: %f\n",Flux->Rncanopy,Flux->Rnsoil);
		printf("albedvegcan->canopy:%f\n",albedvegcan->canopy);
		printf("albedvegcan->soil:%f\n",albedvegcan->soil);
		printf("RL->soil:%f\n",RL->soil);
		printf("RL->air:%f\n",RL->air);
		printf("RL->canopy:%f\n",RL->canopy);
		exit(1);
	}
	/*Soil Flux computed as simple fraction of Rn at the soil */
	Flux->G = cg*(Flux->Rnsoil);
} /*end of getRnG */

/**--------------------------------------------------------------------------**/
/**WIND FUNCTIONS *************************************************************/
/**--------------------------------------------------------------------------**/

double ustar_func(double K, double u, double Zu, double d0m,double z0m,double Psim)
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
}/*end of getwind function */

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

