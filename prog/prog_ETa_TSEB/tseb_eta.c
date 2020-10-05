/*Norman and Kustas 2 source model */
/* code by Andrew French 2002 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <gdal.h>
#include "tseb_eta.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing OpenMP Code------------\n");
	printf( "-----------------------------------------\n");
	printf( "./eta_tseb inLst inNdvi inCanopyHeight inCanopyWidth\n");
	printf( "\tinViewAngle inSolarZenithAngle \n");
	printf( "\tinBUWO inETPOTD\n\n");
	printf( "inBUWO: Bare/urban/water/other = 0,1,2,3\n\n");
	printf( "\tout_EVAPFR_TSEB out_ETA_TSEB out_H_SOIL_TSEB out_H_CANOP_TSEB\n");
	printf( "\tout_LE_SOIL_TSEB out_LE_CANOP_TSEB out_G0_TSEB out_RN_TSEB\n");
	printf( "\tout_LAI_TSEB out_RES_AIR_TSEB out_RES_SOIL_TSEB out_L_MOST_TSEB\n");
	printf( "-----------------------------------------\n");
	return;
}

int main(int argc,char *argv[])
{
	if( argc < 20) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
	char	*inB1		= argv[1]; //LST
	char	*inB2		= argv[2]; //NDVI
	char	*inB3		= argv[3]; //canopy height
	char	*inB4		= argv[4]; //canopy width
	char	*inB5		= argv[5]; //view angle
	char	*inB6		= argv[6]; //solar zenith angle (degrees)
	char	*inB7		= argv[7]; //Bare/Urban/water/other
	char	*inB8		= argv[8]; //ET POT radiative (mm/d)
	//Output files
	char	*evap_frF	= argv[9]; //Output evaporative fraction (-)
	char	*etaF		= argv[10]; //Output ETa (mm/d)
	char	*h_soilF	= argv[11]; //soil sensible heat flux output
	char	*h_canopyF	= argv[12]; //canopy sensible heat flux output
	char	*le_soilF 	= argv[13]; //soil latent heat flux output
	char	*le_canopyF 	= argv[14]; //canopy latent heat flux output
	char	*g0F		= argv[15]; //soil heat flux output
	char	*RnF		= argv[16]; //net radiation output
	char	*laiF		= argv[17]; //leaf area index output
	char	*resist_airF	= argv[18]; //air resistance output
	char	*resist_soilF	= argv[19]; //soil resistance output
	char	*L_mostF	= argv[20]; //Monin-Obukov Length output



	/**GDAL STUFF***************/
	//Loading the input files
	//-----------------------
// 	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//LST
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//NDVI
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//canopy height
	GDALDatasetH hD4 = GDALOpen(inB4,GA_ReadOnly);//canopy width
	GDALDatasetH hD5 = GDALOpen(inB5,GA_ReadOnly);//view angle
	GDALDatasetH hD6 = GDALOpen(inB6,GA_ReadOnly);//solar zenith angle (degrees)
	GDALDatasetH hD7 = GDALOpen(inB7,GA_ReadOnly);//Bare/Urban/water/other
	GDALDatasetH hD8 = GDALOpen(inB8,GA_ReadOnly);//ETPOTD
	if(hD1==NULL||hD2==NULL||hD3==NULL||hD4==NULL||
	  hD5==NULL||hD6==NULL||hD7==NULL||hD8==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	//Loading the file infos
	//----------------------
	GDALDriverH hDr8 = GDALGetDatasetDriver(hD8);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	//Creating output files
	GDALDatasetH hDOut0 = GDALCreateCopy( hDr8, evap_frF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut0 = GDALGetRasterBand(hDOut0,1);
	GDALDatasetH hDOut1 = GDALCreateCopy( hDr8, etaF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut1 = GDALGetRasterBand(hDOut1,1);
	GDALDatasetH hDOut2 = GDALCreateCopy( hDr8, h_soilF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut2 = GDALGetRasterBand(hDOut2,1);
	GDALDatasetH hDOut3 = GDALCreateCopy( hDr8, h_canopyF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut3 = GDALGetRasterBand(hDOut3,1);
	GDALDatasetH hDOut4 = GDALCreateCopy( hDr8, le_soilF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut4 = GDALGetRasterBand(hDOut4,1);
	GDALDatasetH hDOut5 = GDALCreateCopy( hDr8, le_canopyF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut5 = GDALGetRasterBand(hDOut5,1);
	GDALDatasetH hDOut6 = GDALCreateCopy( hDr8, g0F,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut6 = GDALGetRasterBand(hDOut6,1);
	GDALDatasetH hDOut7 = GDALCreateCopy( hDr8, RnF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut7 = GDALGetRasterBand(hDOut7,1);
	GDALDatasetH hDOut8 = GDALCreateCopy( hDr8, laiF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut8 = GDALGetRasterBand(hDOut8,1);
	GDALDatasetH hDOut9 = GDALCreateCopy( hDr8, resist_airF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut9 = GDALGetRasterBand(hDOut9,1);
	GDALDatasetH hDOut10 = GDALCreateCopy( hDr8, resist_soilF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut10 = GDALGetRasterBand(hDOut10,1);
	GDALDatasetH hDOut11 = GDALCreateCopy( hDr8, L_mostF,hD8,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut11 = GDALGetRasterBand(hDOut11,1);
	//Loading the file bands
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);
	GDALRasterBandH hB4 = GDALGetRasterBand(hD4,1);
	GDALRasterBandH hB5 = GDALGetRasterBand(hD5,1);
	GDALRasterBandH hB6 = GDALGetRasterBand(hD6,1);
	GDALRasterBandH hB7 = GDALGetRasterBand(hD7,1);
	GDALRasterBandH hB8 = GDALGetRasterBand(hD8,1);

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int rc, N=nX*nY; //rowxcol processing in Device Memory

	/* Allocate arrays on host: start with INPUT */
	float *lst 		= (float*) malloc(N*sizeof(float));
	float *ndvi 		= (float*) malloc(N*sizeof(float));
	float *canopy_height 	= (float*) malloc(N*sizeof(float));
	float *canopy_width 	= (float*) malloc(N*sizeof(float));
	float *viewangle 	= (float*) malloc(N*sizeof(float));
	float *sunza	 	= (float*) malloc(N*sizeof(float));
	float *buwo		= (float*) malloc(N*sizeof(float));
	/* OUTPUT */
	float *etpotd 		= (float*) malloc(N*sizeof(float));
	float *evap_fr 		= (float*) malloc(N*sizeof(float));
	float *eta 		= (float*) malloc(N*sizeof(float));
	float *h_soil 		= (float*) malloc(N*sizeof(float));
	float *h_canopy		= (float*) malloc(N*sizeof(float));
	float *le_soil 		= (float*) malloc(N*sizeof(float));
	float *le_canopy	= (float*) malloc(N*sizeof(float));
	float *g0		= (float*) malloc(N*sizeof(float));
	float *Rn		= (float*) malloc(N*sizeof(float));
	float *lai		= (float*) malloc(N*sizeof(float));
	float *resist_air	= (float*) malloc(N*sizeof(float));
	float *resist_soil	= (float*) malloc(N*sizeof(float));
	float *L_most		= (float*) malloc(N*sizeof(float));

	/* Read input files through GDAL */
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,lst,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,ndvi,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,canopy_height,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB4,GF_Read,0,0,nX,nY,canopy_width,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB5,GF_Read,0,0,nX,nY,viewangle,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB6,GF_Read,0,0,nX,nY,sunza,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB7,GF_Read,0,0,nX,nY,buwo,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB8,GF_Read,0,0,nX,nY,etpotd,nX,nY,GDT_Float32,0,0);

	meteo		met		={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	tkstruct 	tk		={0.0,0.0,0.0,0.0};
	soilabsemiss 	soilabs_ems	={0.0,0.0,0.0};
	leafabsemiss 	leafabs_ems	={0.0,0.0,0.0,0.0};
	radweights 	radwts		={0.0,0.0,0.8,0.2,0.9,0.1};
	Choud 		choudparms	={0.0,0.0};
	NDVIrng 	ndvi_rng	={0.0,0.0,0.0};
	Cover 		vegcover	={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	refhts 		Z		={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	fluxstruct 	Flux		={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	resistor 	resist		={0.0,0.0};
	CanopyLight 	cpylight	={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
	LngWave 	RL		={0.0,0.0,0.0};
	Albeds 		albedvegcan;

	/**Read variables from input and allocate them to structures**/

 	int nodataval 			= -28768;//No data initialization
// 	double canopy_height_BUW	= 0.1;//Bare soil / Urban / Water canopy_height value
	double const Lmonobfinit	= -1e9;
	met.Rsolar			= 824.0;//Incoming Solar radiation initialization (W/m2)
	met.presmbar			= 860.0;//Atmospheric pressure initialization (mbar)
	tk.air				= 30.2+273.15;//Air-temperature initialization (K)
	met.windspeed			= 2.0;//Wind speed initialiazation (m/s)
	met.rh				= 0.3;//Relative Humidity initialization (-)
	double cG			= 0.25;//cG constant (for g0)
	soilabs_ems.vis			= 0.93;//soil absorptivity emissivity visible
	soilabs_ems.nir			= 0.77;//soil_absorptivity_nir
	soilabs_ems.emisstir		= 0.95;//soil_emissivity_tir
	leafabs_ems.vis			= 0.8;//leaf_absorptivity_visible
	leafabs_ems.nir			= 0.60;//leaf_absorptivity_nir
	leafabs_ems.emisstir		= 0.98;//leaf_emissivity_tir
	leafabs_ems.extinct		= 0.95;//leaf_extinction_tir
	radwts.visdir			= 0.5;//beam_visible_weight_factor
	radwts.nirdir			= 0.5;//beam_nir_weight_factor
	radwts.visdif			= 0.9;//diffuse_visible_weight_factor
	radwts.nirdif			= 0.1;//diffuse_nir_weight_factor
	choudparms.p			= 0.6;//Choudhury_p_exponent
	choudparms.Beta			= 0.5;//Choudhury_beta
	ndvi_rng.min			= -0.2;//NDVI threshold min
	ndvi_rng.max			= 0.6;//NDVI threshold max
	ndvi_rng.baresoil		= 0.2;//NDVI threshold baresoil
	vegcover.green			= 0.5;//Fraction of vegetation that is green
	vegcover.leafangparm		= 1.0;//Leaf Angle Distribution Parameter
	vegcover.clumpfactornadir	= 0.95;//Clumping factor at nadir view
	vegcover.PT			= 1.26;//Priestley-Taylor alpha
	Z.ref				= 10.0;//Z reference height (m)
	Z.refbare			= 0.2;//Z reference height bare soil (m)
	Z.u				= 10.0;//Z wind speed reference height (m)
	Z.t				= 9.0;//Z temperature reference height (m)

	int stabflag = 0;
	int exitflag = 0;
	int iternum;
	int condenseflag;
	double tempval,esat,eslope,rsat,rhodryair;

	double Lmonobfprev;
	double Re,kB;

	/**START PROCESSING**/
	/*compute air density, vapor pressure, cp, esat, specific humidity*/
	richesat_ptr(tk.air,&esat,&eslope);
	met.esat 	= esat;
	met.desat_dtk 	= eslope;
	rsat 		= e2r(esat,met.presmbar);
	met.mixratio 	= met.rh*rsat;
	tempval 	= r2e(met.mixratio,met.presmbar);
	met.ea 		= tempval;

	printf("esat is %f and eslope is %f\n",esat,eslope);
	printf("mixing ratio is %f\n",met.mixratio);
	printf("vapor pressure ea is %f\n",met.ea);

	tempval 	= e_mb2rhoden(met.ea,tk.air);
	met.vapden 	= 0.001*tempval;
	rhodryair 	= 0.001*rhodry(met.vapden,met.presmbar,tk.air);
	met.rhoair 	= e2rhomoist(met.ea,met.presmbar,tk.air);
	tempval 	= mixr2spechum(met.mixratio);
	met.spechum 	= tempval;

	tempval 	= cpd2cp(met.spechum);
	met.cp 		= tempval;

	met.lambdav 	= latenthtvap(k2c(tk.air));
	met.gamma 	= gammafunc(tk.air,met.presmbar,met.cp,met.lambdav);
	/*Finished filling meteorological structure 'met' */

	printf("air temperature(Kelvin): %f\n",tk.air);
	printf("met structure:\n");
	printf("rh %f ea %f mixratio %f spechum %f\n",met.rh,met.ea,met.mixratio,met.spechum);
	printf("vapden %f rhoair %f cp %f esat %f\n",met.vapden,met.rhoair,met.cp,met.esat);

	Z.d0bare 	= 0.01;

	#pragma omp parallel for default(none) \
		private(rc, stabflag, iternum, exitflag, condenseflag, Re, kB, Lmonobfprev, \
		vegcover, Z, choudparms, met, ndvi_rng,\
		soilabs_ems, leafabs_ems, radwts, tk, Flux,resist,cpylight, RL, albedvegcan)\
		shared(N, cG, lst, ndvi, canopy_height, canopy_width, viewangle, sunza, etpotd , buwo,\
		h_soil, h_canopy, le_soil, le_canopy, g0, Rn, lai, resist_air, resist_soil, L_most, nodataval)
	for(rc=0;rc<N;rc++) {
		/* CHECK THE NDVI RANGE INPUT */
		if((lst[rc]*0.02 < 0.00)
		  || (ndvi[rc]*0.0001 < -0.5)
		  || (ndvi[rc]*0.0001 > 1.0)
		  || (sunza[rc]*0.01 == nodataval)
		  || (viewangle[rc]*0.01 == -100)) {
			h_soil[rc] 		= nodataval;
			h_canopy[rc] 	= nodataval;
			le_soil[rc] 	= nodataval;
			le_canopy[rc] 	= nodataval;
			g0[rc] 		= nodataval;
			Rn[rc] 		= nodataval;
			lai[rc] 		= nodataval;
			resist_air[rc] 	= nodataval;
			resist_soil[rc] 	= nodataval;
			L_most[rc]		= nodataval;
		} else {
			/*get solar zenith angle */
			met.solzenangrad 	= deg2rad(sunza[rc]*0.01);
			/*get canopy height*/
			vegcover.canopyheight 	= canopy_height[rc] / 1000.0;
			/*get canopy width*/
			vegcover.canopywidth 	= canopy_width[rc] / 1000.0;
			/*get view angle */
			vegcover.viewangrad	= deg2rad(viewangle[rc]*0.01);
			/*compute clumping factor for given viewangle */
			vegcover.clumpfactor 	= clump_factor(vegcover.clumpfactornadir,vegcover.canopyheight,vegcover.canopywidth,vegcover.viewangrad);
			/*set displacement and roughness lengths */
			Z.d0 	= 0.67 * vegcover.canopyheight;
			Z.z0 	= 0.125 * vegcover.canopyheight;
			/*get fractional cover and lai values*/
			vegcover.frac 	= frac_cover_choud(ndvi_rng.min,ndvi_rng.max,ndvi[rc]*0.0001,choudparms.p);
			vegcover.LAI 	= LAI_choudfunc(vegcover.frac,choudparms.Beta);
			/* printf("fractional cover: %f LAI: %f\n",vegcover.frac,vegcover.LAI);*/
			/*Set visible light diffusion parameter according to LAI value */
			if(vegcover.LAI < 0.5) radwts.Kd = 0.9;
			else if(vegcover.LAI > 2.0) radwts.Kd = 0.7;
			else radwts.Kd = 0.6;
			/*copy remote sensing temperature to tk.composite member (Kelvin) */
			tk.composite 	= lst[rc]*0.02;
			/*initialize canopy temperature */
			if(vegcover.frac < 0.5) tk.canopy = 0.5*(tk.air+tk.composite);
			else tk.canopy = tk.air;
			/*initialize soil temperature */
			if(vegcover.frac < 0.8) component_tempk_soil(vegcover.frac,&tk);
			/*if cover is thick, cant rely on getting good soil temperature */
			/* from repartitioning tir data, so set it equal to composite value */
			else tk.soil = tk.composite;
			/*guess stability condition */
			/*set stabflag to 1 for unstable conditions */
			Z.L = Lmonobfinit;
			if(tk.composite > tk.air) stabflag = 1;
			else stabflag = 0;
			/*if unstable conditions, compute Businger-Dyer psi and phi functions */
			/*  otherwise zero out the phi functions*/
			/* printf("Computing stability correction functions.\n");*/
			/*   phiandpsi(&Z,&vegcover);*/
			if(stabflag == 1) xandpsi(&Z,&vegcover);
			else stabphipsi(&Z);
			/*compute light conditions if there is a canopy */
			/* select % cover as threshold */
			/* printf("fractional cover: %f\n",vegcover.frac);*/
			if(vegcover.frac > 0.1 && vegcover.canopywidth != nodataval) {
				/*compute soil and canopy albedo values */
				/*first compute canopy reflectivities and transmissivities*/
				/* for vis and nir under direct and diffuse light */
				canopyrho(&met,&vegcover,&radwts,&leafabs_ems,&soilabs_ems,&cpylight);
				/*compute reflectivity of soil and canopy*/
				/* also canopy transmissivity */
				rhosoil2albedo(&soilabs_ems,&albedvegcan);
				rhocpy2albedo(&cpylight,&albedvegcan,&radwts);
			} else {
				/* compute light conditions where no canopy exists */
				/*set canopy reflectivitie to zero and transmissivities to 1.0 */
				/*   printf("light where no canopy\n"); */
				cpylight.rhovisdir = 0.0;
				cpylight.rhonirdir = 0.0;
				cpylight.rhovisdif = 0.0;
				cpylight.rhonirdif = 0.0;
				cpylight.tauvisdir = 1.0;
				cpylight.taunirdir = 1.0;
				cpylight.tauvisdif = 1.0;
				cpylight.taunirdif = 1.0;
				cpylight.tautir = 1.0;
				rhosoil2albedo(&soilabs_ems,&albedvegcan);
			} /*end of else */
			/*BEGIN ITERATION FOR UNSTABLE CONDITIONS */
			iternum = 0;
			exitflag = 0;
			while(exitflag == 0) {
				/* || stabflag == 1) {*/
				iternum++;
				/*   printf("iternum is %d\n",iternum);*/
				if((buwo[rc]>=0.0) || (buwo[rc]<=2.0) ||
				  (ndvi[rc]*0.0001 < -1.0) ||
				  (ndvi[rc]*0.0001 <= ndvi_rng.baresoil))
				{
					/*if true then have a one layer case could be bare soil, urban or water */
					/*     printf("in one layer routine.\n"); */
					/*compute wind speed and air resistance */
					getwindbare(&met,&Z,&resist,&tk);
					/*     printf("for bare, resist.air %f resist.soil %f\n",resist.air,resist.soil);*/
					/*compute turbulent fluxes from bare soil; also check for condensation*/
					onelayer(&Flux, &tk, cG, &met, &albedvegcan, &soilabs_ems, &leafabs_ems, &resist, &RL);
					/*     exitflag = 1;*/
				} else {
					/*in this case there are two layers, soil and canopy */
					/*compute resistance of air, wind speeds at canopy top and soil surface */
					/* printf("in two layer routine.\n");*/
					getwind(&met,&Z,&vegcover,&resist);
					/* printf("two layers: resist.air %f resist.soil: %f\n",resist.air,resist.soil);*/
					/* printf("met.usoil: %f met.windspeed: %f met.ustar: %f\n",met.usoil,met.windspeed,met.ustar);*/
					/*compute long wave radiation from soil, sky and canopy */
					getrls(&met,&tk,&soilabs_ems,&leafabs_ems,&RL);
					/*compute net radiation components and soil flux */
					getRnG(&cpylight, &albedvegcan, &RL, &met, &Flux, cG, &vegcover);
					printf("RL.soil %f RL.air %f RL.canopy %f\n",RL.soil,RL.air, RL.canopy);
					printf("Rn: %f Rncanopy: %f Rnsoil: %f\n",Flux.Rntotal,Flux.Rncanopy,Flux.Rnsoil);
					/*compute turbulent flux components */
					twolayer(&tk,&Flux,&met,&resist,&vegcover);
					/*check for computed condensation condition */
					/* if it exist,it is unrealistic, dont allow */
					/*force LE to be at least zero on soil and canopy */
					condenseflag = nocondense(&Flux,&tk,&resist,&met,&vegcover);
				} /*end of else */
				/* sum soil and canopy fluxes of each kind */
				Flux.Htotal  = (Flux.Hsoil) + (Flux.Hcanopy);
				Flux.LEtotal = (Flux.LEsoil) + (Flux.LEcanopy);
				/*revise Monin-Obukhov stability length */
				Lmonobfprev = Z.L;
				monobfs(&met,&tk,&Flux,&Z);
				/*   printf("Z.L : %f Z.Lprevious: %f\n",Z.L,Lmonobfprev);*/
				/*check if iteration still needed */
				/*   printf("Ldiff: %f\n",Z.L-Lmonobfprev);*/
				/*   printf("current exitflag value is %d\n",exitflag);*/
				if((((Z.L)-Lmonobfprev ) < 0.001) && ((Z.L)-Lmonobfprev) > -0.001) exitflag = 1;/* printf("exitflag set to one, small difference\n");*/
				else {
					if((Z.L) > 0.0 && (Lmonobfprev > 0.0)) {
						exitflag = 1; /* printf("set exitflag to one since both L's positive\n");*/
					} else {
						if(iternum > MAXITER) {
							exitflag = 1;
							/* printf("set exitflag to one, exceeded MAXITER \n");*/
						} else {
							exitflag = 0;
							xandpsi(&Z,&vegcover);
						}
				    /*end of last if */
					} /*end of second else */
				} /*end of first else */
			/*  printf("end of while loop, exitflag is: %d\n",exitflag);*/
			} /*end of while */
			/*Do stable conditions */
			if(stabflag == 0) {
				/*set Psi values to zero */
				/*stable case doesnt need iteration */
				stabphipsi(&Z);
				/*need to consider both two layer and one layer cases */
				if((buwo[rc]>=0.0) ||(buwo[rc]<=1.0))/*bare & urban*/
				{
					/*if true then have a one layer case */
					/*could be bare soil or urban, do water differently */
					/*compute wind speed and air resistance */
					getwindbare(&met,&Z,&resist,&tk);
					/*compute turbulent fluxes from bare soil; also check for condensation;*/
					onelayer(&Flux, &tk, cG, &met, &albedvegcan, &soilabs_ems, &leafabs_ems, &resist, &RL);
				} else {
					if((buwo[rc]==2.0))/*open water case */
					{
						getwindbare(&met,&Z,&resist,&tk);
						/*compute G and Rn for open water */
						radiatewaters(&tk,&Flux,&met,&albedvegcan);
						/*compute Reynolds number for water */
						Re =  reynolds(&tk,&met,&Z);
						Z.z0h = (Z.z0)*7.48*exp(-2.46*pow(Re,0.25));
						kB = log((Z.z0)/(Z.z0h));
						resist.air = ((log(((Z.t)-(Z.d0))/(Z.z0)))+kB)/((met.ustar)*VONKARMAN);
						hfluxresist_soil(&tk,&resist,&Flux,&met);
						Flux.LEsoil = (Flux.Rnsoil)-(Flux.G)-(Flux.Hsoil);
						Flux.Hcanopy = 0.0;
						Flux.LEcanopy = 0.0;
						Flux.Htotal = Flux.Hsoil;
						Flux.LEtotal = Flux.LEsoil;
						/* N.B.-- member designation 'soil' here means 'water'!! */
						/*end of open water case */
					} else {
						/*do stable two layer case */
						/*computation is same as for unstable two layer case, except that the stability functions are not used */
						getwind(&met,&Z,&vegcover,&resist);
						/*compute long wave radiation from soil, sky and canopy */
						getrls(&met,&tk,&soilabs_ems,&leafabs_ems,&RL);
						getRnG(&cpylight,&albedvegcan,&RL,&met,&Flux,cG,&vegcover);
						twolayer(&tk,&Flux,&met,&resist,&vegcover);
						condenseflag = nocondense(&Flux,&tk,&resist,&met,&vegcover);
						/*end of stable two layer case */
					}
				}
			} /* end of if stabflag==0, ie stable conditions exist */
			h_soil[rc] 		= Flux.Hsoil;
			h_canopy[rc] 	= Flux.Hcanopy;
			le_soil[rc] 	= Flux.LEsoil;
			le_canopy[rc] 	= Flux.LEcanopy;
			g0[rc] 		= Flux.G;
			Rn[rc] 		= Flux.Rntotal;
			lai[rc] 		= vegcover.LAI;
			resist_air[rc] 	= resist.air;
			resist_soil[rc] 	= resist.soil;
			L_most[rc]		= Lmonobfinit ;
			Z.L 			= Lmonobfinit ;
		} /*end of else condition where data are deemed good */
	}/*end of rc loop */
	#pragma omp barrier
	/* Write output file through GDAL */
	GDALRasterIO(hBOut0,GF_Write,0,0,nX,nY,evap_fr,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut1,GF_Write,0,0,nX,nY,eta,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut2,GF_Write,0,0,nX,nY,h_soil,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut3,GF_Write,0,0,nX,nY,h_canopy,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut4,GF_Write,0,0,nX,nY,le_soil,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut5,GF_Write,0,0,nX,nY,le_canopy,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut6,GF_Write,0,0,nX,nY,g0,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut7,GF_Write,0,0,nX,nY,Rn,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut8,GF_Write,0,0,nX,nY,lai,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut9,GF_Write,0,0,nX,nY,resist_air,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut10,GF_Write,0,0,nX,nY,resist_soil,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut11,GF_Write,0,0,nX,nY,L_most,nX,nY,GDT_Float32,0,0);
	/* Free the memory */
	free(lst);
	free(ndvi);
	free(canopy_height);
	free(canopy_width);
	free(viewangle);
	free(sunza);
	free(buwo);
	free(etpotd);
	free(evap_fr);
	free(eta);
	free(h_soil);
	free(h_canopy);
	free(le_soil);
	free(le_canopy);
	free(g0);
	free(Rn);
	free(lai);
	free(resist_air);
	free(resist_soil);
	free(L_most);
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hD4);
	GDALClose(hD5);
	GDALClose(hD6);
	GDALClose(hD7);
	GDALClose(hD8);
	GDALClose(hDOut0);
	GDALClose(hDOut1);
	GDALClose(hDOut2);
	GDALClose(hDOut3);
	GDALClose(hDOut4);
	GDALClose(hDOut5);
	GDALClose(hDOut6);
	GDALClose(hDOut7);
	GDALClose(hDOut8);
	GDALClose(hDOut9);
	GDALClose(hDOut10);
	GDALClose(hDOut11);
	return(EXIT_SUCCESS);
} /*end of main() */