
#include<stdio.h>
#include<omp.h>
#include "gdal.h"
#include "sebal_eta.h"


void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./eta inNDVI inAlbedo\n");
	printf( "\tinB8 inB14 inB15 inB16 inB17\n");
	printf( "\toutEVAPFR outETA outDTAIR outTHETA\n");
	printf( "\ttsw doy roh_w u@2m iteration\n");
	printf( "-----------------------------------------\n");
	printf( "inB1\t\tModis NDVI 1Km\n");
	printf( "inB2\t\tModis Albedo 1Km\n");
	printf( "inB8\t\tModis LST day 1Km\n");
	printf( "inB14\t\tDigital Elevation Model 1Km [m]\n");
	printf( "inB15\t\tModis Diurnal Averaged Net Radiation RNETD 1Km [W/m2]\n");
	printf( "inB16\t\tModis Satellite overpass net radiation RNET 1Km [W/m2]\n");
	printf( "inB17\t\tModis Satellite overpass soil heat flux G0 1Km [W/m2]\n");

	printf( "outEVAPFR\tEvaporative Fraction output [-]\n");
	printf( "outETA\t\tActual ET output [mm/d]\n");
	printf( "outDTAIR\t\tDTair output [K]\n");
	printf( "outTHETA\t\tsoil moisture output [cm3/cm3]\n");

	printf( "tsw\t\tAtmospheric single-way transmissivity [-]\n");
	printf( "doy\t\tDay of Year [1-366]\n");
	printf( "roh_w\t\tBulk density of water [kg/m3]\n");
	printf( "u@2m\t\tWind Speed at 2 meters height [m/s]\n");
	printf( "iteration\tNumber of SEBAL h0 iterations (3-10)\n");
	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 15 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------

	char *inB1 		= argv[1]; //NDVI
	char *inB2	 	= argv[2]; //Albedo
	char *inB8		= argv[3]; //LST
	char *inB14		= argv[4]; //DEM
	char *inB15		= argv[5]; //RNETD
	char *inB16		= argv[6]; //RNET
	char *inB17		= argv[7]; //G0

	char *evapfrF	 	= argv[8];
	char *etaF	 	= argv[9];
	char *dtairF	 	= argv[10];
	char *thetaF	 	= argv[11];

	float tsw		= atof( argv[12] );
	int doy			= atoi( argv[13] );
	float roh_w	 	= atof( argv[14] );
	float u2m	 	= atof( argv[15] );
	int iteration 		= atoi( argv[16] );

	printf("\ntsw\t= %7.2f\nroh_w\t= %7.2f\nu@2m\t= %7.2f\n\n",tsw, roh_w, u2m);
	//Loading the input files
	//-----------------------
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//NDVI
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//Albedo
	GDALDatasetH hD8 = GDALOpen(inB8,GA_ReadOnly);//LST
	GDALDatasetH hD14 = GDALOpen(inB14,GA_ReadOnly);//DEM
	GDALDatasetH hD15 = GDALOpen(inB15,GA_ReadOnly);//RNETD
	GDALDatasetH hD16 = GDALOpen(inB16,GA_ReadOnly);//RNET
	GDALDatasetH hD17 = GDALOpen(inB17,GA_ReadOnly);//G0

	if(hD1==NULL||hD2==NULL||hD8==NULL||hD14==NULL||
	  hD15==NULL||hD16==NULL||hD17==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}

	//Loading the file infos
	//----------------------
	GDALDriverH hDr14 = GDALGetDatasetDriver(hD14);

	//Creating output file
	//--------------------
	//Evaporative fraction
	GDALDatasetH hDOut4 = GDALCreateCopy( hDr14, evapfrF,hD14,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut4 = GDALGetRasterBand(hDOut4,1);
	//ETa
	GDALDatasetH hDOut5 = GDALCreateCopy( hDr14, etaF,hD14,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut5 = GDALGetRasterBand(hDOut5,1);

	//DTair
	GDALDatasetH hDOut6 = GDALCreateCopy( hDr14, dtairF,hD14,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut6 = GDALGetRasterBand(hDOut6,1);
	//Theta
	GDALDatasetH hDOut7 = GDALCreateCopy( hDr14, thetaF,hD14,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut7 = GDALGetRasterBand(hDOut7,1);

	//Loading the file bands
	//----------------------
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//NDVI
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//Albedo
	GDALRasterBandH hB8 = GDALGetRasterBand(hD8,1);
	GDALRasterBandH hB14 = GDALGetRasterBand(hD14,1);
	GDALRasterBandH hB15 = GDALGetRasterBand(hD15,1);
	GDALRasterBandH hB16 = GDALGetRasterBand(hD16,1);
	GDALRasterBandH hB17 = GDALGetRasterBand(hD17,1);

// 	printf("Passed 1\n");
	//Loading the data rowxrow
	//------------------------
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N=nX*nY;

// 	printf("Passed 3\n");
	float *mat1 = (float *) malloc(sizeof(float)*N);
	float *mat2 = (float *) malloc(sizeof(float)*N);
	float *mat8 = (float *) malloc(sizeof(float)*N);
	float *mat14 = (float *) malloc(sizeof(float)*N);

// 	printf("Passed 4\n");
	float *matOut3 = (float *) malloc(sizeof(float)*N);
	float *matOut4 = (float *) malloc(sizeof(float)*N);
	float *matOut5 = (float *) malloc(sizeof(float)*N);
	float *matOut6 = (float *) malloc(sizeof(float)*N);
	float *matOut7 = (float *) malloc(sizeof(float)*N);

	float *mat15 = (float *) malloc(sizeof(float)*N);
	float *mat16 = (float *) malloc(sizeof(float)*N);
	float *mat17 = (float *) malloc(sizeof(float)*N);

	int rowcol;
	float tempk, etpotd, g0;
	float ndvi_max=0.0, albedo_max=0.001, albedo_min=0.9;//init values
	double dt;//dtair map out

// 	printf("Passed 5\n");
	//NDVI 1Km
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,mat1,nX,nY,GDT_Float32,0,0);
	//Albedo 1Km
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,mat2,nX,nY,GDT_Float32,0,0);
	//LST 1Km
	GDALRasterIO(hB8,GF_Read,0,0,nX,nY,mat8,nX,nY,GDT_Float32,0,0);
	//DEM 1Km
	GDALRasterIO(hB14,GF_Read,0,0,nX,nY,mat14,nX,nY,GDT_Float32,0,0);
	//RNETD 1Km
	GDALRasterIO(hB15,GF_Read,0,0,nX,nY,mat15,nX,nY,GDT_Float32,0,0);
	//RNET 1Km
	GDALRasterIO(hB16,GF_Read,0,0,nX,nY,mat16,nX,nY,GDT_Float32,0,0);
	//G0 1Km
	GDALRasterIO(hB17,GF_Read,0,0,nX,nY,mat17,nX,nY,GDT_Float32,0,0);
	//---------------------------
	// Pre-Processing
	//---------------------------
	#pragma omp parallel for default(none) \
		private(rowcol, tempk, etpotd)\
		shared(N, nX, nY, roh_w, tsw, doy,\
			ndvi_max,albedo_min,albedo_max, \
			mat1,mat2,mat8, mat15, \
			matOut3 )
	for(rowcol=0;rowcol<N;rowcol++){
// 		for(col=0;col<nX;col++){
		if(mat2[rowcol]==-28768||mat8[rowcol]==28768||mat2[rowcol]<=0.001){
			/*SKIP IT*/
			matOut3[rowcol] = 0.0;
		} else {
		if (mat1[rowcol]*0.0001>ndvi_max&&mat1[rowcol]*0.0001<0.98)
			ndvi_max = mat1[rowcol]*0.0001;
		if (mat2[rowcol]*0.001>albedo_max&&mat2[rowcol]*0.001<0.9)
			albedo_max = mat2[rowcol]*0.001;
		if (mat2[rowcol]*0.001<albedo_min&&mat2[rowcol]*0.001>0.001)
			albedo_min = mat2[rowcol]*0.001;
		tempk = mat8[rowcol] * 0.02;
// 		tempka = tempk - deltat;
//		tempka = tempk - 5;
		etpotd = et_pot_day( mat15[rowcol], tempk, roh_w );
		matOut3[rowcol] = etpotd;
		}
	}
	#pragma omp barrier

	printf("\nAlbedo_min\t= %7.5f\nAlbedo_max\t= %7.5f\n",albedo_min,albedo_max);
	//Accessing the data rowxrow
	//---------------------------
	// Dry/wet pixel seek
	//---------------------------
	/* Pick up wet and dry pixel values */
	float Rn, h0, h0_max=0.0, dem, albedo, t0dem ;
	float tempk_wet, tempk_dry;
	float tempk_min=400.0, tempk_max=0.0;
	float t0dem_min=400.0, t0dem_max=0.0;
	float Rn_dry, g0_dry;
	float Rn_wet, g0_wet;
	float t0dem_dry, dem_dry;
	float t0dem_wet=400;
	int rowcol_dry;
	int rowcol_wet;
	/*START Temperature minimum search */
	/* THREAD 1 */
	/*This is correcting for un-Earthly temperatures*/
	/*It finds when histogram is actually starting to pull up...*/
	int i, temp;
	int peak1, peak2, peak3;
	int i_peak1, i_peak2, i_peak3;
	int bottom1a, bottom1b;
	int bottom2a, bottom2b;
	int bottom3a, bottom3b;
	int i_bottom1a, i_bottom1b;
	int i_bottom2a, i_bottom2b;
	int i_bottom3a, i_bottom3b;
	int histogramT[400];
	for (i=0;i<400;i++){
		histogramT[i]=0;
	}
	/****************************/
	/* Process pixels histogram */
	for(rowcol=0;rowcol<N;rowcol++){
// 		for(col=0;col<nX;col++){
		if(mat2[rowcol]==-28768||mat8[rowcol]==28768||mat16[rowcol]==-28768||mat17[rowcol]==-28768){
			/*skip it*/
		} else {
			temp = (int) (mat8[rowcol] * 0.02);
			if(temp>250){
				histogramT[temp]=histogramT[temp]+1.0;
			}
		}
	}
// 	}
// 	for(i=0;i<400;i++)
// 	 	printf("%i %i\n",i,histogramT[i]);
// 	printf("Histogram of Temperature map");
// 	printf(" (if it has rogue values to clean)\n");
	peak1=0;
	peak2=0;
	peak3=0;
	i_peak1=0;
	i_peak2=0;
	i_peak3=0;
	bottom1a=100000;
	bottom1b=100000;
	bottom2a=100000;
	bottom2b=100000;
	bottom3a=100000;
	bottom3b=100000;
	i_bottom1a=1000;
	i_bottom1b=1000;
	i_bottom2a=1000;
	i_bottom2b=1000;
	i_bottom3a=1000;
	i_bottom3b=1000;
	for(i=0;i<400;i++){
		/* Search for highest peak of dataset (2) */
		/* Highest Peak */
		if(histogramT[i]>peak2){
			peak2 = histogramT[i];
			i_peak2=i;
		}
	}
	int stop=0;
	for(i=i_peak2;i>5;i--){
		if(((histogramT[i]+histogramT[i-1]+histogramT[i-2]+histogramT[i-3]+histogramT[i-4])/5)<histogramT[i]&&stop==0){
			bottom2a = histogramT[i];
			i_bottom2a = i;
		} else if(((histogramT[i]+histogramT[i-1]+histogramT[i-2]+histogramT[i-3]+histogramT[i-4])/5)>histogramT[i]&&stop==0){
			/*Search for peaks of datasets (1)*/
			peak1 = histogramT[i];
			i_peak1=i;
			stop=1;
		}
	}
	stop=0;
	for(i=i_peak2;i<395;i++){
		if(((histogramT[i]+histogramT[i+1]+histogramT[i+2]+histogramT[i+3]+histogramT[i+4])/5)<histogramT[i]&&stop==0){
			bottom2b = histogramT[i];
			i_bottom2b = i;
		} else if(((histogramT[i]+histogramT[i+1]+histogramT[i+2]+histogramT[i+3]+histogramT[i+4])/5)>histogramT[i]&&stop==0){
			/*Search for peaks of datasets (3)*/
			peak3 = histogramT[i];
			i_peak3=i;
			stop=1;
		}
	}
	/* First histogram lower bound */
	for(i=250;i<i_peak1;i++){
		if(histogramT[i]<bottom1a){
			bottom1a = histogramT[i];
			i_bottom1a = i;
		}
	}
	/* First histogram higher bound */
	for(i=i_peak2;i>i_peak1;i--){
		if(histogramT[i]<=bottom1b){
			bottom1b = histogramT[i];
			i_bottom1b = i;
		}
	}
	/* Third histogram lower bound */
	for(i=i_peak2;i<i_peak3;i++){
		if(histogramT[i]<bottom3a){
			bottom3a = histogramT[i];
			i_bottom3a = i;
		}
	}
	/* Third histogram higher bound */
	for(i=399;i>i_peak3;i--){
		if(histogramT[i]<bottom3b){
			bottom3b = histogramT[i];
			i_bottom3b = i;
		}
	}
	printf("\nbottom1a:\t[%i]\t=> %i\n",i_bottom1a, bottom1a);
	printf("peak1:\t\t[%i]\t=> %i\n",i_peak1, peak1);
	printf("bottom1b:\t[%i]\t=> %i\n",i_bottom1b, bottom1b);
	printf("bottom2a:\t[%i]\t=> %i\n",i_bottom2a, bottom2a);
	printf("peak2:\t\t[%i]\t=> %i\n",i_peak2, peak2);
	printf("bottom2b:\t[%i]\t=> %i\n",i_bottom2b, bottom2b);
	printf("bottom3a:\t[%i]\t=> %i\n",i_bottom3a, bottom3a);
	printf("peak3:\t\t[%i]\t=> %i\n",i_peak3, peak3);
	printf("bottom3b:\t[%i]\t=> %i\n\n",i_bottom3b, bottom3b);
	if(i_peak1<250){
		if(i_bottom2a<273.15) i_peak1=273.15;
		else i_peak1=i_bottom2a;
		printf("Corrected: i_peak1:\t\t%i\n",i_peak1);
	}
	if(i_peak3<250||i_peak3>350){
		i_peak3=i_bottom2b;
		printf("Corrected: i_peak3:\t\t%i\n",i_peak3);
	}
	#pragma omp parallel for default(none) \
		private(rowcol, tempk, Rn, g0, albedo, dem, h0, t0dem) \
		shared(N, t0dem_min, t0dem_max, tempk_min, tempk_max, \
			tempk_wet, tempk_dry, t0dem_wet, t0dem_dry, \
			g0_wet, g0_dry, Rn_wet, Rn_dry, dem_dry, \
			rowcol_wet, rowcol_dry, h0_max, \
			i_peak3, i_peak1, albedo_min, albedo_max, \
			mat2, mat8, mat14, mat16, mat17)
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat2[rowcol]==-28768||mat8[rowcol]==28768||
		  mat17[rowcol]==-28768||isnan(mat17[rowcol])||
		  mat16[rowcol]==-28768||isnan(mat16[rowcol])){
			/* do nothing */
		} else {
			tempk = mat8[rowcol] * 0.02;
			Rn = mat16[rowcol];
			g0 = mat17[rowcol];
			albedo = mat2[rowcol]*0.001;
			dem = mat14[rowcol];
			h0=Rn-g0;
			t0dem = tempk + 0.00627 * dem;
			if(t0dem<t0dem_min&&albedo<0.15&&
			  t0dem>274.0&&Rn>10.0&&g0>1.0){
				t0dem_min=t0dem;
				t0dem_wet=t0dem;
				tempk_min=tempk;
				tempk_wet=tempk;
				Rn_wet=Rn;
				g0_wet=g0;
				rowcol_wet=rowcol;
			}
// 			if(tempk>=(float)i_peak1-7.0&&tempk<(float)i_peak1+10.0&&
// 			  t0dem>274.0&&t0dem<t0dem_min&&g0>1.0){
// 				tempk_min=tempk;
// 				tempk_wet=tempk;
// 				t0dem_min=t0dem;
// 				t0dem_wet=t0dem;
// 				Rn_wet=Rn;
// 				g0_wet=g0;
// 				rowcol_wet=rowcol;
// 			}
			if(t0dem>t0dem_max&&h0>h0_max&&t0dem>t0dem_min+1.0&&Rn>0.0){
				t0dem_max=t0dem;
				t0dem_dry=t0dem;
				tempk_max=tempk;
				tempk_dry=tempk;
				Rn_dry=Rn;
				g0_dry=g0;
				dem_dry=dem;
				rowcol_dry=rowcol;
			}
			if(tempk>=(float)i_peak3-0.0&&tempk<(float)i_peak3+7.0&&
			h0>10.0&&h0>h0_max&&g0>1.0&&Rn>0.0&&albedo>0.5*albedo_max){
				tempk_max=tempk;
				tempk_dry=tempk;
				t0dem_max=t0dem;
				t0dem_dry=t0dem;
				Rn_dry=Rn;
				g0_dry=g0;
				h0_max=h0;
				dem_dry=dem;
				rowcol_dry=rowcol;
			}
		}
	}
	#pragma omp barrier
	printf("\ntempk_min\t= %7.2f\ntempk_max\t= %7.2f\n\n",tempk_min,tempk_max);
	printf("rowcol_wet\t= %d\n",rowcol_wet);
	printf("rowcol_dry\t= %d\n",rowcol_dry);
	printf("tempk_wet\t= %7.2f\n",tempk_wet);
	printf("g0_wet\t\t= %7.2f\n",g0_wet);
	printf("Rn_wet\t\t= %7.2f\n",Rn_wet);
	printf("LE_wet\t\t= %7.2f\n",Rn_wet-g0_wet);
	printf("tempk_dry\t= %7.2f\n",tempk_dry);
	printf("dem_dry\t\t= %7.2f\n",dem_dry);
	printf("t0dem_dry\t= %7.2f\n",t0dem_dry);
	printf("rnet_dry\t= %7.2f\n",Rn_dry);
	printf("g0_dry\t\t= %7.2f\n",g0_dry);

	// Energy Balance Processing
	//---------------------------
	float score=0.0;
	int score_max=0;
	#pragma omp parallel for default(none) \
		private (rowcol, h0, dt) \
		shared (N,iteration,ndvi_max,Rn_dry,g0_dry,doy,score,score_max, \
			 t0dem_wet,t0dem_dry,u2m,dem_dry,tempk_wet,tempk_dry, \
			mat1,mat2,mat8,mat14,mat15,mat16,mat17, \
			matOut4,matOut5,matOut6,matOut7)
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat2[rowcol]==-28768||mat8[rowcol]==28768||isnan(mat17[rowcol])||
		mat17[rowcol]<=0.0||mat16[rowcol]<=0.0||
		mat14[rowcol]<=-100.0||mat14[rowcol]>9000.0||
		mat8[rowcol]*0.02<250.0){
			/* Do Nothing */
			matOut4[rowcol] = -28768;
			matOut5[rowcol] = -28768;
		} else if(mat8[rowcol]*0.02<=273.15&&mat8[rowcol]*0.02>250.0){
			//Sublimation
			matOut4[rowcol] = 0.01;
			score=score+1.0;
			matOut5[rowcol] = et_a(mat15[rowcol], matOut4[rowcol], mat8[rowcol]*0.02);
			score_max++;
		} else {
			/* Calculate sensible heat flux */
			h0 = sensi_h(iteration,
				mat8[rowcol]*0.02 + 0.00627 * mat14[rowcol],
				mat1[rowcol]*0.0001,
				ndvi_max,
				mat14[rowcol],
				Rn_dry,
				g0_dry,
				t0dem_dry,
				t0dem_wet,
				u2m,
				dem_dry,
				doy,
				&dt);
			matOut6[rowcol] = dt;
			matOut4[rowcol] = evap_fr(mat16[rowcol], mat17[rowcol], h0);
			matOut7[rowcol] = soilmoisture(matOut4[rowcol]);
			if(matOut4[rowcol]<1.0&&matOut4[rowcol]>0.0) score=score+1.0;
			matOut5[rowcol] = et_a(mat15[rowcol], matOut4[rowcol], mat8[rowcol]*0.02);
			score_max++;
		}
	}
	#pragma omp barrier
	FILE *f;
	f=fopen("log.dat","a");
	fprintf(f,"%d\t%7.2f\n",doy,score*100/score_max);
	fclose(f);
	printf("Score\t=%7.2f\n",score*100/score_max);
	GDALRasterIO(hBOut4,GF_Write,0,0,nX,nY,matOut4,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut5,GF_Write,0,0,nX,nY,matOut5,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut6,GF_Write,0,0,nX,nY,matOut6,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut7,GF_Write,0,0,nX,nY,matOut7,nX,nY,GDT_Float32,0,0);
// 	printf("serial: Free memory\n");
	if( mat1 != NULL ) free( mat1 );
	if( mat2 != NULL ) free( mat2 );
	if( mat8 != NULL ) free( mat8 );
	if( mat14 != NULL ) free( mat14 );
	if( mat15 != NULL ) free( mat15 );
	if( mat16 != NULL ) free( mat16 );
	if( mat17 != NULL ) free( mat17 );
	if( matOut3 != NULL ) free( matOut3 );
	if( matOut4 != NULL ) free( matOut4 );
	if( matOut5 != NULL ) free( matOut5 );
	if( matOut6 != NULL ) free( matOut6 );
	if( matOut7 != NULL ) free( matOut7 );

	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD8);
	GDALClose(hD14);
	GDALClose(hDOut4);
	GDALClose(hDOut5);
	GDALClose(hDOut6);
	GDALClose(hDOut7);
	return(EXIT_SUCCESS);
}

