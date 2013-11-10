#define SPECHEAT_DRY	1004.67
#define STAND_PRESMBAR	1013.25
#define STAND_DENSITY	1.225
#define Rd		287.04
#define Rv		461.50
#define MolWtH2O	18.016
#define MolWtDryAir	28.966
#define GRAV		9.81
#define c1		3.741774922e-22
#define c2		0.0143876912
#define STEFANBOLTZ	5.67e-8
#define VONKARMAN	0.4
#define MAXITER		4
#define NUMCHAR		120
#define PI		3.1415927
/*STEFANBOLTZ in W/(m^2-Kelvin^4) */


// STRUCTURES FIRST USED IN READING INPUT PARAMETER FILE

/*meteorological values, except for air temperature*/
typedef struct {
	double rhoair;
	double cp;
	double esat;
	double ea;
	double rh;
	double lambdav;
	double gamma;
	double vapden;
	double spechum;
	double mixratio;
	double windspeed;
	double presmbar;
	double Rsolar;
	double solzenangrad;
	double desat_dtk;
	double usoil;
	double ucanopy;
	double ustar;
} meteo;

/*temperature values of soil, canopy, air and composite */
typedef  struct {
	double soil;
	double canopy;
	double composite;
	double air;
} tkstruct;

/*Soil absorptivity and emissivity properties */
typedef struct {
	double vis;
	double nir;
	double emisstir;
} soilabsemiss;

/*Leaf absorptivity and emissivity properties */
typedef struct {
	double vis;
	double nir;
	double emisstir;
	double extinct;
} leafabsemiss;

/* Extinction coefficient parameters for beam and diffuse radiation */
/*ellipsoidal leaf angle distribution for Kbe */
/*spectral weight factor for computing albedo */
typedef struct {
	double Kbe;
	double Kd;
	double visdir;
	double nirdir;
	double visdif;
	double nirdif;
} radweights;

/* Choudhury parameters for vegetation cover */
typedef struct {
	double p;
	double Beta;
} Choud;
  
typedef struct {
	double min;
	double max;
	double baresoil;
} NDVIrng;

typedef struct {
	double frac;
	double LAI;
	double clumpfactor;
	double clumpfactornadir;
	double PT;
	double green;
	double viewangrad;
	double leafangparm;
	double canopyheight;
	double canopywidth;
	double leafwidth;
} Cover;

/*Measurement heights, roughnesses, stability functions */
typedef struct {
	double ref;
	double refbare;
	double t;
	double u;
	double z0;
	double z0h;
	double d0;
	double d0bare;
	double L;
	double zeta_u;
	double zeta_tt;
	double zeta_ref;
	double phih;
	double phihref;
	double phim;
	double phimref;
	double Psih;
	double Psihref;
	double Psim;
	double Psimref;
	double X;
	double Xt;
	double Xref;
} refhts; 
// END OF FIRST USED IN READING INPUT PARAMETER FILE

typedef struct {
	double Hsoil;
	double Hcanopy;
	double Htotal;
	double LEsoil;
	double LEcanopy;
	double LEtotal;
	double G;
	double Rnsoil;
	double Rncanopy;
	double Rntotal;
} fluxstruct;

/*resistances */
typedef struct {
	double soil;
	double air;
} resistor;

/*Reflectivities and Transmissivities of vegetation canopy*/
typedef struct {
	double rhovisdir;
	double rhonirdir;
	double rhovisdif;
	double rhonirdif;
	double tauvisdir;
	double taunirdir;
	double tauvisdif;
	double taunirdif;
	double tausolar;
	double tautir;
} CanopyLight;

/*Short Wave Radiation/Albedo */
typedef struct {
	double soil;
	double canopy;
} Albeds;


/*Long wave radiation */
typedef struct {
	double air;
	double soil;
	double canopy;
} LngWave;


//MISSING prototypes
void richesat_ptr(double tk, double *esatmb_p, double *eslope_p);
double r2e(double r, double Pmbar);
double e_mb2rhoden(double e_mbar, double tkair);
double rhodry(double e_mbar, double p_mbar, double tkelvin);
double mixr2spechum(double mixr);
double cpd2cp(double q);
double latenthtvap(double tcair);
double k2c(double tkelvin);
double deg2rad(double degrees);
double gammafunc(double tkair,double pressmbar,double cpJKkg,double lambdav);
void component_tempk_soil(double fracveg,tkstruct *tkstrucptr);
void rhocpy2albedo(CanopyLight *cpylight,Albeds *albedvegcan,radweights *radwts);
// END OF MISSING prototypes

double e2r(double embar, double Pmbar);
double e2rhomoist(double embar,double presmbar,double tk);
double frac_cover_choud(double ndvi_min,double ndvi_max,double ndvi_obs,double choud_p);
double LAI_choudfunc(double frac_cover,double Beta);

void getrls(meteo *met, tkstruct *tk,soilabsemiss *soilabs_ems,leafabsemiss *leafabs_ems,LngWave *RL);
void ehfluxes(double green, double PT, meteo *met,fluxstruct *Flux);

void tempresist_soil(fluxstruct *Fluxstrptr,tkstruct *tkstrptr,resistor *resist,meteo *met);
void tempresist_can(fluxstruct *Fluxstrptr,tkstruct *tkstrptr,resistor *resist,meteo *met);
void hfluxresist_soil( tkstruct *tkstrucptr,resistor *resist,fluxstruct *Fluxstrucptr,meteo *met);
void hfluxresist_canopy( tkstruct *tkstrucptr,resistor *resist,fluxstruct *Fluxstrucptr,meteo *met);
void sauers(tkstruct *tk, meteo *met, resistor *resist);

void phiandpsi(refhts *Z,Cover *vegcover);

void stabphipsi(refhts *Z);

void xandpsi(refhts *Z,Cover *vegcover);

double kbefunc(double x, double Psirad);

void canopyrho(meteo *met,Cover *vegcover,radweights *radwts,leafabsemiss *leafabs_ems,soilabsemiss *soilabs_ems,CanopyLight *cpylight);

double canopytran(double absorptivity,double Kextinct,double LAI, double clumpfactor,double rhonothor,double rhosoil);

void rhosoil2albedo(soilabsemiss *soilabs_ems,Albeds *albedvegcan);

void getwind(meteo *met,refhts *Z,Cover *vegcover,resistor *resist);

void getwindbare(meteo *met,refhts *Z,resistor *resist, tkstruct *tk);

void getRnG(CanopyLight *cpylight,Albeds *albedvegcan,LngWave *RL,meteo *met,fluxstruct *Flux,double cg, Cover *vegcover);

double ustar_func(double K, double u, double Zu, double d0m,double z0m,double Psim);

double ra_func_orig(double Zt, double d0m, double z0m,double Psih, double U, double K, double Psim, double Zu);

double ra_func(double Zt, double d0m, double z0m,double Psih, double ustar, double K);

double ra_funcbareRe(double Zt, double Zref, double d0m,double z0m,double Psih,double Psir,double ustar,double K, double rho,double tkair, double tkrad);

double ucpy_func(double u, double hc, double d0m,double z0m, double K, double ustar);

void twolayer(tkstruct *tk,fluxstruct *Flux,meteo *met,resistor *resist,Cover *vegcover);

void onelayer(fluxstruct *Flux,tkstruct *tk,double cg,meteo *met,Albeds *albedvegcan,soilabsemiss *soilabs_ems,leafabsemiss *leafabs_ems,resistor *resist, LngWave *RL);

int nocondense(fluxstruct *Flux,tkstruct *tk,resistor *resist,meteo *met,Cover *vegcover);

void monobfs(meteo *met,tkstruct *tk,fluxstruct *Flux,refhts *Z);

void radiatewaters(tkstruct *tk,fluxstruct *Flux, meteo *met,Albeds *albedvegcan);
double reynolds(tkstruct *tk, meteo *met,refhts *Z);

double clump_factor(double clumpnadir,double viewangrad,double canopy_ht,double canopy_wdth);

