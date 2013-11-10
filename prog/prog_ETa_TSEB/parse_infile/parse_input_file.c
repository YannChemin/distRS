#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NUMCHAR 120
#define PI 3.1415927

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
	double viewangrad;//init from where?
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

double deg2rad(double degrees)
{
	double radval;
	radval = degrees*PI/180.0;
	return(radval);
}

double rad2deg(double radianval)
{
	double degval;
	degval = radianval*180.0/PI;
	return(degval);
}

/* Celsius to Kelvin */
double c2k(double tcelsius)
{
	double tkelvin;
	tkelvin = tcelsius+273.15;
	return(tkelvin);
}

/*grab control information from file designated on command line*/
int main(int argc, char *argv[])
{
	char infofil[NUMCHAR]="text.txt";
	char dumrec[NUMCHAR];
	char *tok;
	char ch;
	char *chp;
	char **next;
	ch = 'x';
	chp = &ch;
	next = &chp;
	
	int base = 10, i, lstrlen;
	static char inputsep[] = "=\n";
	static char whitespace[] = " \t\f\r\v\n";

	meteo met		= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	tkstruct tk 		= {0.0,0.0,0.0,0.0};
	soilabsemiss soilabs_ems= {0.0,0.0,0.0};
	leafabsemiss leafabs_ems= {0.0,0.0,0.0,0.0};
	radweights radwts	= {0.0,0.0,0.8,0.2,0.9,0.1};
	Choud choudparms	= {0.0,0.0};
	NDVIrng ndvi_rng	= {0.0,0.0,0.0};
	Cover vegcover		= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	refhts Z		= {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

	/* IN ORDER OF READING */
	char	inputimagefil[NUMCHAR],outputimagefil[NUMCHAR];
	int	numbands,numpixels,numlines;
	double 	tkscale,tkoffset;
	double 	ndviscale,ndvioffset;
	int 	numlandusetypes=255;//Number of land use types to read (initialized at 255)
	/*allocate space for land use data */
	char **landusename = (char **) malloc(numlandusetypes*sizeof(char *));
	if(landusename == NULL) {
		printf("ould not allocate space for landusename, exiting!\n");
		exit(EXIT_FAILURE);
	}
	double *canopy_height = (double *) malloc(numlandusetypes*sizeof(double));
	if(canopy_height==NULL) {
	  printf("Could not allocate space for canopy heights, exiting!\n");
	  exit(EXIT_FAILURE);
	}	
	int *landusecode = (int *) malloc(numlandusetypes*sizeof(int));
	if(landusecode==NULL) {
		printf("Could not allocate space for land use codes, exiting!\n");
		exit(EXIT_FAILURE);
	}
	
	double tempval;//Dummy variable
	double cG;//cG constant (g0)
	
	FILE *finfop;

	if((finfop = fopen(infofil,"r"))==NULL) {
	  printf("Cannot open information file!!");
	  exit(1);
	};
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	sscanf(tok,"%s",inputimagefil);
	printf("\nInput image file:\t%s\n",inputimagefil);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	sscanf(tok,"%s",outputimagefil);
	printf("Output image file:\t%s\n",outputimagefil);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	numbands = (int) strtol(tok,next,base);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	numpixels = (int) strtol(tok,next,base);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	numlines = (int) strtol(tok,next,base);
	printf("\n# input bands:\t%d\tpixels:\t%d\tlines:\t\t%d\n",numbands,numpixels,numlines);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,whitespace);
	tkscale = strtod(tok,next);
	tok = strtok(NULL,whitespace);
	tkoffset = strtod(tok,next);
	printf("TK scale:\t%.2f\toffset:\t%.2f\n",tkscale,tkoffset);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,whitespace);
	ndviscale = strtod(tok,next);
	tok = strtok(NULL,whitespace);
	ndvioffset = strtod(tok,next);
	printf("NDVI scale:\t%.3f\toffset:\t%.2f\n",ndviscale,ndvioffset);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	numlandusetypes = (int) strtol(tok,next,base);
	printf("\nNumber of land use types to read:\t%d\n",numlandusetypes);

	/* read header for landuse information */
	fgets(dumrec,NUMCHAR,finfop);
	printf("-----------------------------------------------\n");
	printf("Land_use_code\tCanopy_height(m) Land_Use_Name\n");
	printf("-----------------------------------------------\n");
	/*loop through land use information */
	for(i=0;i<numlandusetypes;i++) {
		fgets(dumrec,NUMCHAR,finfop);
		tok = strtok(dumrec,inputsep);
		landusecode[i] = (int) strtol(tok,next,base);
		tok = strtok(NULL,inputsep);
		canopy_height[i] = strtod(tok,next);
		tok = strtok(NULL,inputsep);
		lstrlen = (int) strlen(tok);
		landusename[i] = (char *) malloc(lstrlen*sizeof(char));
		if(landusename[i]==NULL) {
			printf("Could not allocate landusename for type %d,exiting!\n",i);
			exit(EXIT_FAILURE);
		}
		strcpy(landusename[i],tok);
		printf("\t%d\t%.2f\t\t%s\n",landusecode[i],canopy_height[i],landusename[i]);
	  }
	printf("-----------------------------------------------\n");
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	printf("\nsolar zenith angle (degrees):\t\t%.2f\n",tempval);
	met.solzenangrad = deg2rad(tempval);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	met.Rsolar = strtod(tok,next);
	printf("Incoming solar radiation (W/m^2):\t%.2f\n",met.Rsolar);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	met.presmbar = strtod(tok,next);
	printf("surface pressure (mbar):\t\t%.2f\n",met.presmbar);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	printf("surface air temperature is:\t\t%.2f\n",tempval);
	tk.air = c2k(tempval);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	met.windspeed = strtod(tok,next);
	printf("surface wind speed (m/s):\t\t%.2f\n",met.windspeed);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	met.rh = strtod(tok,next);
	if(met.rh >1.0) {
		printf("Relative humidity value specified greater than 1.0!!\n");
		printf(" It should be a fraction of 1.0, correct and re-run, now exiting!!\n");
		exit(EXIT_FAILURE);
	}

	printf("Surface humidity (as fraction of 1.0):\t%.2f\n",met.rh);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	cG = strtod(tok,next);
	printf("cG constant:\t\t\t\t%.2f\n",cG);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	printf("\n-----------------------------------------------------\n");
	printf("\t\t\t\tvis\tnir\ttir\n");
	printf("-----------------------------------------------------\n");
	printf("soilabs_ems\t\t\t%.2f",tempval);
	soilabs_ems.vis = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	printf("\t%.2f\n",tempval);
	soilabs_ems.nir = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	soilabs_ems.emisstir = tempval;

	printf("Soil absorptivity emissivity\t%.2f\t%.2f\t%.2f\n", soilabs_ems.vis, soilabs_ems.nir, soilabs_ems.emisstir);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	leafabs_ems.vis = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	leafabs_ems.nir = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	leafabs_ems.emisstir = tempval;


	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	leafabs_ems.extinct = tempval;

	printf("Leaf absorptivity extinction");
	printf("\t%.2f\t%.2f\t%.2f\n",leafabs_ems.vis,leafabs_ems.nir,leafabs_ems.extinct);
	printf("-----------------------------------------------------\n");


	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	radwts.visdir = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	radwts.nirdir = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	radwts.visdif = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	radwts.nirdif = tempval;

	printf("\nSpectral Weight Factors for beam and diffuse radiation\n");
	printf("-----------------------------------------------------\n");
	printf("\t\t\t\tvis\tnir\n");
	printf("-----------------------------------------------------\n");
	printf("Beam:\t\t\t\t%.2f\t%.2f\n",radwts.visdir,radwts.nirdir);
	printf("Diffuse:\t\t\t%.2f\t%.2f\n",radwts.visdif,radwts.nirdif);
	printf("-----------------------------------------------------\n");

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	choudparms.p = tempval;
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	choudparms.Beta = tempval;
	printf("\nChoudhury params p and Beta:\t%.2f\t%.2f\n",choudparms.p,choudparms.Beta);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	ndvi_rng.min = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	ndvi_rng.max = tempval;

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	ndvi_rng.baresoil = tempval;
	printf("NDVI threshold min and max:\t%.2f\t%.2f\n",ndvi_rng.min,ndvi_rng.max);
	printf("NDVI threshold bare soil:\t\t%.2f\n",ndvi_rng.baresoil);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.green = tempval;
	printf("Fraction of vegetation that is green:\t%.2f\n",vegcover.green);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.leafwidth = tempval;
	printf("Leaf width (m)\t\t\t\t%.2f\n",vegcover.leafwidth);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.canopywidth = tempval;
	printf("Canopy width (m)\t\t\t%.2f\n",vegcover.canopywidth);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.leafangparm = tempval;
	printf("Leaf Angle Distribution Parameter:\t%.2f\n",vegcover.leafangparm);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.clumpfactornadir = tempval;
	printf("Clumping factor at nadir view:\t\t%.2f\n",vegcover.clumpfactornadir);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	vegcover.PT = tempval;
	printf("Priestley-Taylor alpha:\t\t\t%.2f\n",vegcover.PT);

	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	Z.ref = tempval;
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	Z.refbare = tempval;
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	Z.u = tempval;
	fgets(dumrec,NUMCHAR,finfop);
	tok = strtok(dumrec,inputsep);
	tok = strtok(NULL,inputsep);
	tempval = strtod(tok,next);
	Z.t = tempval;
	printf("Heights:\n\tref\t\t%.2f\n\trefbare\t\t%.2f\n\twind\t\t%.2f\n\ttemperature\t%.2f\n",Z.ref,Z.refbare,Z.u,Z.t);
	fclose(finfop);
}