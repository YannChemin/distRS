#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tseb_eta.h"
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
	tkv = (1.0+0.61*spechum)*tkv;
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
      rad = (emiss*c1/((pow(lam,5))*M_PI*term))*1000.0;
      return(rad);
}

double rad2tk(double rad,double lambda,double emiss)
{
      double term,lam,tk;
      lam = lambda*1e-6;
      term = ((emiss*c1)/(pow(lam,5)*M_PI*rad*0.001)) + 1.0;
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
double gammafunc(double tkair,double pressmbar,
	     double cpJKkg,double lambdav)
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
	double emissair;
	double albedowater = 0.1;
	/*Brutsaert formula for atmosperic emissivity */
	emissair   = 1.24*pow((met->ea/tk->air),(1.0/7.0));
	Flux->Rntotal = (1.0-albedowater)*(met->Rsolar)*
	  emissair*STEFANBOLTZ*pow((tk->air),4.0);
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
void getRnG(CanopyLight *cpylight,Albeds *albedvegcan,LngWave *RL,meteo *met,fluxstruct *Flux,double cg, Cover *vegcover,int *refinfo)
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

		printf("lct: %d pct: %d\n",refinfo[0],refinfo[1]);
		printf("ltc: %d lndvi: %d\n",refinfo[2],refinfo[3]);
		exit(1);
	}
	/*Soil Flux computed as simple fraction of Rn at the soil */
	Flux->G = cg*(Flux->Rnsoil);
} /*end of getRnG */
