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
	double clumpf;
	double p,D;
	D = canopy_ht/canopy_wdth;
	p = 3.80-0.46*D;
	if(p < 1.0) p = 1.0;
	else if(p > 3.34) p= 3.34;
	clumpf = clumpnadir/(clumpnadir+(1.0-clumpnadir)*exp(-2.2*pow(viewangrad,p)));
	return(clumpf);
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
double rhocpyfunc(double rhonothor, double rhosoil,double absorptivity,double K,double LAI,double Omega)
{
	double expterm,fracterm,rho_sparse;
	expterm = exp(-2.0*sqrt(absorptivity)*K*LAI*Omega);
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
	double emissair;
	/*Brutsaert formula for atmosperic emissivity */
	#if 0
	from idl program
	Rncanopy = (1.0-tautherm)*(rlsky+rlsoil-2.0*rlcanopy)+(1.0-tausolar)*(1.0-albedocanopy)*Rs
	Rnsoil = (tautherm*rlsky)+((1.0-tautherm)*rlcanopy)-rlsoil + tausolar*(1.0-albedosoil)*Rs
	#endif
	emissair   = 1.24*pow((met->ea/tk->air),(1.0/7.0));
	RL->soil   = soilabs_ems->emisstir*stefanboltz*pow(tk->soil,4);
	RL->canopy = leafabs_ems->emisstir*stefanboltz*pow(tk->canopy,4);
	RL->air    = emissair*stefanboltz*pow(tk->air,4);
	#if 0
	printf("tkair: %f\n",tk->air);
	printf("tksoil: %f\n",tk->soil);
	printf("tkcanopy: %f\n",tk->canopy);
	printf("emissair: %f\n",emissair);
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
	double Bos=10.0;
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
} /*end of nocondense */

/*compute fluxes for one layer condition */
/* this function already checks for condensation, so dont do it later*/
void onelayer(fluxstruct *Flux,tkstruct *tk, double cg, meteo *met,Albeds *albedvegcan, soilabsemiss *soilabs_ems, leafabsemiss *leafabs_ems, resistor *resist, LngWave *RL)
{
	/*  double sigma= 5.67e-8;*/
	double Bos=10.0;
	double totsoilresist;
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
	double rhovisdif,rhonirdif;
	double rhovisdir,rhonirdir;
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

