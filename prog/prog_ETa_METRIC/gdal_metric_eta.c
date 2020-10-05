#include<stdio.h>
#include<omp.h>
#include<math.h>
#include "gdal.h"
#include "metric_eta.h"
#include "ogr_srs_api.h"
#include "proj_api.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./metric_eta inNdvi inLai inLst inAlb inDem\n");
	printf( "\toutMetric_eta outMetric_evapfr outMetric_dtair outMetric_theta\n");
	printf( "-----------------------------------------\n");
	printf( "\tdoy Ta rh u z h eto_alf kc iteration\n");
	printf( "\t[-mproj/-mcolrow/-mauto]\n");
	printf( "\nBelow are wet/dry pixels modes\n");
	printf( "---------------------------------------------\n");
	printf( "-mproj projXwet ProjYwet projXdry projYdry\t Manual wet/dry pixel mode (projected)\n");
	printf( "-mcolrow Xwet Ywet Xdry Ydry\t Manual wet/dry pixel mode (NOT projected)\n");
	printf( "-mauto\t Automatic seek wet/dry pixel mode (Careful!)\n\n");

	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 18 ) {
		usage();
		return 1;
	}
	int 	mproj=0, mcolrow=0, mauto=0;
	double	projXwet,projYwet,projXdry,projYdry;
	double	Xwet,Ywet,Xdry,Ydry;
	int	col_wet, col_dry;
	int	row_wet, row_dry;

	char *inB1 		= argv[1]; //NDVI
	char *inB2	 	= argv[2]; //LAI
	char *inB3	 	= argv[3]; //LST
	char *inB4	 	= argv[4]; //ALBEDO
	char *inB5	 	= argv[5]; //DEM
	char *metric_etaF	= argv[6];
	char *metric_evapfrF	= argv[7];
	char *metric_dtairF	= argv[8];
	char *metric_thetaF	= argv[9];
	int doy		= atoi(argv[10]); // Day Of Year [0-366]
	double ta	= atof(argv[11]); // Air Temperature (K)
	double rh	= atof(argv[12]); // Relative Humidity (-)
	double u	= atof(argv[13]); // wind speed (m/s) at z height
	double z	= atof(argv[14]); // z height (m) of anemometer
	double h	= atof(argv[15]); // h (m) of vegetation under anemometer
	double eto_alf	= atof(argv[16]); // Reference ET of Alfalfa
	double kc	= atof(argv[17]); // Reference ET associated crop coefficient
	int iteration	= atoi(argv[18]); // Iteration number for H

	//MDBA Farm A
// 	double phase_max=sin(2*3.1415927*(doy+365/3.3)/365);
// 	double tmax	=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_max);
// 	double esat	=6.11*exp(17.27*tmax/(tmax+237.3));
// 	double eact	=rh*esat/100;
// 	double phase_min=sin(2*3.1415927*(doy+365/3.5)/365);
// 	double tmin	=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_min);
// 	ta		=tmin+(tmax-tmin)*0.8;
	double tsw=0;
	printf("NDVI=%s LAI=%s LST=%s ALBEDO=%s DEM=%s Out=%s\n",inB1,inB2,inB3,inB4,inB5,metric_etaF);
	printf("doy=%i Ta=%f u=%f z=%f h=%f eto_alf=%f kc=%f\n",doy,ta,u,z,h,eto_alf,kc);
	int c;
	for (c = 19; c <argc; c++) {
//		printf("argv[%i] = %s\n",c,argv[c]);
		/* Manual wet / dry input, projected */
		if (!strcmp(argv[c],"-mproj")) {
			mproj=1;
			mcolrow=0;
			mauto=0;
			projXwet = atof( argv[c+1] );
			projYwet = atof( argv[c+2] );
			projXdry = atof( argv[c+3] );
			projYdry = atof( argv[c+4] );
//			printf("projXYwet=(%f,%f) projXYdry=(%f,%f)\n",projXwet,projYwet,projXdry,projYdry);
		}
		/* Manual wet / dry input row / col */
		if (!strcmp(argv[c],"-mcolrow")) {
			mproj=0;
			mcolrow=1;
			mauto=0;
			col_wet	= atoi( argv[c+1] );
			row_wet	= atoi( argv[c+2] );
			col_dry	= atoi( argv[c+3] );
			row_dry	= atoi( argv[c+4] );
		}
		/* Automatic wet / dry input row / col */
		if (!strcmp(argv[c],"-mauto")) {
			mproj=0;
			mcolrow=0;
			mauto=1;
		}
	}
	if (mproj==0 && mcolrow==0 && mauto==0){
		printf("Choose a wet/dry pixel mode please\n");
		exit(1);
	}
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//NDVI
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//LAI
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//LST
	GDALDatasetH hD4 = GDALOpen(inB4,GA_ReadOnly);//ALBEDO
	GDALDatasetH hD5 = GDALOpen(inB5,GA_ReadOnly);//DEM
	if(hD1==NULL||hD2==NULL||hD3==NULL
	  ||hD4==NULL||hD5==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	GDALDriverH hDr5 = GDALGetDatasetDriver(hD5);
	//Evapfr out
	GDALDatasetH hDOut0 = GDALCreateCopy( hDr5, metric_evapfrF,hD5,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut0 = GDALGetRasterBand(hDOut0,1);
	//ETa out
	GDALDatasetH hDOut = GDALCreateCopy( hDr5, metric_etaF,hD5,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	//dTair out
	GDALDatasetH hDOut1 = GDALCreateCopy( hDr5, metric_dtairF,hD5,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut1 = GDALGetRasterBand(hDOut1,1);
	//theta out
	GDALDatasetH hDOut2 = GDALCreateCopy( hDr5, metric_thetaF,hD5,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut2 = GDALGetRasterBand(hDOut2,1);
// 	//latitude temporary skeleton
	GDALDatasetH hDLat = GDALCreateCopy( hDr5, "latitude",hD5,FALSE,options,NULL,NULL);
	GDALRasterBandH hBLat = GDALGetRasterBand(hDLat,1);

	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//NDVI
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//LAI
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//LST
	GDALRasterBandH hB4 = GDALGetRasterBand(hD4,1);//ALBEDO
	GDALRasterBandH hB5 = GDALGetRasterBand(hD5,1);//DEM

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N=nX*nY;

	float *mat1 = (float *) malloc(sizeof(float)*N);
	float *mat2 = (float *) malloc(sizeof(float)*N);
	float *mat3 = (float *) malloc(sizeof(float)*N);
	float *mat4 = (float *) malloc(sizeof(float)*N);
	float *mat5 = (float *) malloc(sizeof(float)*N);
	float *matOut2 = (float *) malloc(sizeof(float)*N);
	float *matOut1 = (float *) malloc(sizeof(float)*N);
	float *matOut0 = (float *) malloc(sizeof(float)*N);
	float *matOut = (float *) malloc(sizeof(float)*N);
	//Latitude matrix
	float *matLat = (float *) malloc(sizeof(float)*N);

	//tmp scanlines
	float *l1 = (float *) malloc(sizeof(float)*nX);
	float *l2 = (float *) malloc(sizeof(float)*nX);
	float *l3 = (float *) malloc(sizeof(float)*nX);
	float *l4 = (float *) malloc(sizeof(float)*nX);
	float *l5 = (float *) malloc(sizeof(float)*nX);

	float e0, kin, lin, lout, lnet, rnet, g_0, z_0m, h0 ;
	float metriceta, metricevapfr, metricdtair, pmeto, metrictheta;
	int i, row, col, rowcol;
	double a[10]={0.0}, b[10]={0.0};
// 	a = (double *) malloc(sizeof(double)*10);
// 	b = (double *) malloc(sizeof(double)*10);
// 	for (i=0;i<10;i++){
// 		a[i]=0.0;
// 		b[i]=0.0;
// 	}
// 	printf("a[0]=%f\tb[0]=%f\n",a[0],b[0]);
	//Calculation of effective wind speed (initilasation of a unique point based ustar)
	float ustar_0 = ustar0(u, z, h);
	//Calculation of surface roughness of momentum
	z_0m=0.12*h;
	float u200 = ustar_0*log(200/z_0m)/0.41;
	//Calculation of aerodynamic resistance to heat flux momentum
	float rah_0 = rah0(ustar_0);//initialsation of rah calculation
	//Seed the wet and dry pixels variables
	double dem_wet=0,rnet_wet,g0_wet;
	double dem_dry=2000,rnet_dry,g0_dry;
	//LOAD DATA MATRICES
	//NDVI
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,mat1,nX,nY,GDT_Float32,0,0);
	//LAI
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,mat2,nX,nY,GDT_Float32,0,0);
	//LST
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,mat3,nX,nY,GDT_Float32,0,0);
	//ALBEDO
	GDALRasterIO(hB4,GF_Read,0,0,nX,nY,mat4,nX,nY,GDT_Float32,0,0);
	//DEM
	GDALRasterIO(hB5,GF_Read,0,0,nX,nY,mat5,nX,nY,GDT_Float32,0,0);
// 	PROCESS DRY AND WET PIXELS
	//PROJ4 things for reproject lat/long pix into row/col
	projPJ proj4;
	projUV in, out;
	char	*proj;
	char    *pszWKT;
	OGRSpatialReferenceH hSRS;
	hSRS = OSRNewSpatialReference( NULL );
	double coef;

	int 	temp;
	double 	geomx[6]={0.0};

	double tempk_min, tempk_max;
	double t0dem_min, t0dem_max;
	double tempk_dry, tempk_wet;
	double t0dem_dry, t0dem_wet;
	double h_dry;

	double dailyN, t0dem, tadem;

	//LATITUDE MATRIX FILLING
	if((GDALGetProjectionRef( hD5 )) != NULL && strlen(GDALGetProjectionRef( hD5 ))>0){
		pszWKT = (char *) GDALGetProjectionRef(hD5);
		printf( "Projection Info\n\t%s\n", pszWKT );
		OSRImportFromWkt(hSRS, &pszWKT);
		OSRExportToProj4(hSRS, &proj);
		GDALGetGeoTransform(hD5,geomx);
		/* Do Nothing */
 		printf( "Origin (ULx,ULy) = (%.6f,%.6f)\n", geomx[0], geomx[3] );
 		printf( "Pixel Size = (%.6f,%.6f)\n", geomx[1], geomx[5] );
 		printf( "Rot0 = (%.6f,%.6f)\n", geomx[2], geomx[4] );
	} else {
		printf("ERROR: Projection acquisition problem from SRTM\n");
		exit(1);
	}
// 	printf("Passed 2\n");
	//if(pj_is_latlong(proj)){
		for(row=0;row<nY;row++){
			#pragma omp parallel for default(none) \
				private(col) shared( row, geomx, nX, l1)
			for(col=0;col<nX;col++){
				l1[col] = geomx[3]+geomx[4]*col+geomx[5]*row;
			}
			#pragma omp barrier
			GDALRasterIO(hBLat,GF_Write,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
		}
		//LATITUDE MATRIX FILLING
		GDALRasterIO(hBLat,GF_Read,0,0,nX,nY,matLat,nX,nY,GDT_Float32,0,0);
	//} else {
	//	printf("ERROR: Projection is NOT LAT/LONG\n");
	//	exit(1);
	//}

	//Accessing the data rowxrow
	if(mcolrow||mproj){
		if(mcolrow){
			row = row_dry;
			col = col_dry;
		} else if (mproj){
			if((GDALGetProjectionRef( hD1 )) != NULL &&
				strlen(GDALGetProjectionRef( hD1 ))>0){
				pszWKT = (char *) GDALGetProjectionRef(hD1);
			//	printf( "%s\n", pszWKT );
				OSRImportFromWkt(hSRS, &pszWKT);
				OSRExportToProj4(hSRS, &proj);
			}
			proj4 = pj_init_plus(proj);
			if(pj_is_latlong(proj4)){
				Xwet = projXwet;
				Ywet = projYwet;
				Xdry = projXdry;
				Ydry = projYdry;
//				printf("Xwet=%f Ywet=%f Xdry=%f Ydry=%f\n",Xwet,Ywet,Xdry,Ydry);
			}else{
				//reproject wet pixels
				in.u = projXwet;
				in.v = projYwet;
				in.u *= DEG_TO_RAD;
				in.v *= DEG_TO_RAD;
				out = pj_inv(in, proj4);
				Xwet = out.u;
				Ywet = out.v;
				//reproject dry pixels
				in.u = projXdry;
				in.v = projYdry;
				in.u *= DEG_TO_RAD;
				in.v *= DEG_TO_RAD;
				out = pj_inv(in, proj4);
				Xdry = out.u;
				Ydry = out.v;
			}
			if(GDALGetGeoTransform(hD1,geomx)==CE_None){
				/* Do Nothing */
// 				printf( "Origin (ULx,ULy) = (%.6f,%.6f)\n", geomx[0], geomx[3] );
// 				printf( "Pixel Size = (%.6f,%.6f)\n", geomx[1], geomx[5] );
// 				printf( "Rot0 = (%.6f,%.6f)\n", geomx[2], geomx[4] );
			} else {
				printf("ERROR: Projection acquisition problem from Band1\n");
				exit(1);
			}
//			printf( "Origin(ULx,ULy)= (%.6f,%.6f)\n", geomx[0], geomx[3] );
//			printf( "Pixel Size= (%.6f,%.6f)\n", geomx[1], geomx[5] );
//			printf( "Rot1= (%.6f,%.6f)\n", geomx[2], geomx[4] );
			if(geomx[2]<=0.0001||(-geomx[4])<=0.0001){
				col=(Xdry-geomx[0])/geomx[1];
				row=(geomx[3]-Ydry)/(-geomx[5]);
//				printf("col1=%i row1=%i\n",col,row);
			}else {
				coef = geomx[5]/geomx[4];
//				printf("coef_dry=%f\n",coef);
				col = (coef*Xdry-coef*geomx[0]-Ydry+geomx[3])/(coef*geomx[1]-geomx[2]);
//				printf("col=%i\n",col);
				row = (Xdry - (geomx[0] + geomx[1] * col))/geomx[2];
//				printf("row=%i\n",row);
			}
// 			projXdry = geomx[0] + geomx[1] * col_dry + geomx[2] * row_dry;
// 			projYdry = geomx[3] + geomx[4] * col_dry + geomx[5] * row_dry;
		}
		/* Collect T0Dem_dry */
		GDALRasterIO(hB3,GF_Read,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
		tempk_dry = l1[col] * 0.02;
		GDALRasterIO(hB5,GF_Read,0,row,nX,1,l2,nX,1,GDT_Float32,0,0);
		t0dem_dry = tempk_dry - 0.00625*l2[col] ;
		GDALRasterIO(hB1,GF_Read,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB2,GF_Read,0,row,nX,1,l2,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB4,GF_Read,0,row,nX,1,l4,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB5,GF_Read,0,row,nX,1,l5,nX,1,GDT_Float32,0,0);
		// Surface emissivity calculations (e0=f(ndvi,lai))
		e0=e_0(l1[col]*0.0001,l2[col]*0.1);
		//Incoming longwave radiation (Lin). To be derived locally at CSU
		tsw = 0.75 + 2 * pow(10,-5)*l5[col];
		lin = Lin(tsw, ta);
// 		printf("lin=%f\n", lin);
		//Outgoing longwave radiation (Lout=f(e0,lst))
		lout=Lout(e0,tempk_dry);
		//Net longwave radiation (Lnet)
		lnet=Lnet(lout,lin);
		//Solar shortwave incoming irradiance
		kin=Kin(doy,geomx[3]+geomx[4]*col+geomx[5]*row,tsw);
		//Net Radiation (Rnet = f(alb,lnet,kin)
		rnet_dry=Rnet(l4[col]*0.001,lnet,kin);
		//calculation of g_0 = f(rnet,lst,alb,ndvi)
		g0_dry=g0(rnet_dry,tempk_dry,l4[col]*0.001,l1[col]*0.0001);
		h_dry = rnet_dry - g0_dry ;
		row_dry=row;
		col_dry=col;
		if(mcolrow){
			row = row_wet;
			col = col_wet;
		} else if (mproj){
			if(geomx[2]<=0.0001||(-geomx[4])<=0.0001){
				col=(Xwet-geomx[0])/geomx[1];
				row=(geomx[3]-Ywet)/(-geomx[5]);
//				printf("col=%i row=%i\n",col,row);
			}else {
				coef = geomx[5]/geomx[4];
				col = (coef*Xwet-coef*geomx[0]-Ywet+geomx[3])/(coef*geomx[1]-geomx[2]);
				row = (Xwet - (geomx[0] + geomx[1] * col))/geomx[2];
			}
// 			projXwet = geomx[0] + geomx[1] * col_wet + geomx[2] * row_wet;
// 			projYwet = geomx[3] + geomx[4] * col_wet + geomx[5] * row_wet;
		}

		/* Collect T0Dem_wet */
		GDALRasterIO(hB3,GF_Read,0,row,nX,1,l3,nX,1,GDT_Float32,0,0);
		tempk_wet = l3[col] * 0.02;
		GDALRasterIO(hB5,GF_Read,0,row,nX,1,l5,nX,1,GDT_Float32,0,0);
		t0dem_wet = tempk_wet - 0.00625*l5[col] ;
		GDALRasterIO(hB1,GF_Read,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB2,GF_Read,0,row,nX,1,l2,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB4,GF_Read,0,row,nX,1,l4,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB5,GF_Read,0,row,nX,1,l5,nX,1,GDT_Float32,0,0);
		// Surface emissivity calculations (e0=f(ndvi,lai))
		e0=e_0(l1[col]*0.0001,l2[col]*0.1);
		//Incoming longwave radiation (Lin). To be derived locally at CSU
		tsw = 0.75 + 2 * pow(10,-5)*l5[col];
		lin = Lin(tsw, ta);
// 		printf("lin=%f\n", lin);
		//Outgoing longwave radiation (Lout=f(e0,lst))
		lout=Lout(e0,tempk_wet);
		//Net longwave radiation (Lnet)
		lnet=Lnet(lout,lin);
		//Solar shortwave incoming irradiance
		kin=Kin(doy,geomx[3]+geomx[4]*col+geomx[5]*row,tsw);
		//Net Radiation (Rnet = f(alb,lnet,kin)
		rnet_wet=Rnet(l4[col]*0.001,lnet,kin);
		//calculation of g0 = f(rnet,lst,alb,ndvi)
		g0_wet=g0(rnet_wet,tempk_wet,l4[col]*0.001,l1[col]*0.0001);
		row_wet=row;
		col_wet=col;
	} else {
		//---------------------------
		// Dry/wet pixel AUTOMATIC seek
		//---------------------------
// 		printf("Started Heuristic\n");
		tempk_min=400.0;
		tempk_max=0.0;
		t0dem_min=400.0;
		t0dem_max=0.0;
		col_dry = 0;
		row_dry = 0;
		col_wet = 0;
		row_wet = 0;
		/* Pick up wet and dry pixel values */
 		double h0, dem, albedo, t0dem, tempk ;
		double h0_min=1000.0;
		double h0_max=0.0;
		/*START Temperature minimum search */
		/*This is correcting for un-Earthly temperatures*/
		/*It finds when histogram is actually starting to pull up...*/
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
		#pragma omp parallel for default(none) \
		private(rowcol, temp)\
		shared(N, mat3, mat5, histogramT)
		for(rowcol=0;rowcol<N;rowcol++){
			temp = (int) (mat3[rowcol]*0.02-0.00625*mat5[rowcol]);
			if(temp>200||mat3[rowcol]!=-28768){
				histogramT[temp]=histogramT[temp]+1.0;
			}
		}
		#pragma omp barrier
//		printf("Histogram of Temperature map");
//		printf(" (if it has rogue values to clean)\n");
		//int peak1, peak2, peak3;
		//int i_peak1, i_peak2, i_peak3;
		peak1=0;
		peak2=0;
		peak3=0;
		i_peak1=0;
		i_peak2=0;
		i_peak3=0;
		//int bottom1a, bottom1b;
		//int bottom2a, bottom2b;
		//int bottom3a, bottom3b;
		//int i_bottom1a, i_bottom1b;
		//int i_bottom2a, i_bottom2b;
		//int i_bottom3a, i_bottom3b;
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
//		printf("bottom1a: [%i]=>%i\n",i_bottom1a, bottom1a);
//		printf("peak1: [%i]=>%i\n",i_peak1, peak1);
//		printf("bottom1b: [%i]=>%i\n",i_bottom1b, bottom1b);
//		printf("bottom2a: [%i]=>%i\n",i_bottom2a, bottom2a);
//		printf("peak2: [%i]=>%i\n",i_peak2, peak2);
//		printf("bottom2b: [%i]=>%i\n",i_bottom2b, bottom2b);
//		printf("bottom3a: [%i]=>%i\n",i_bottom3a, bottom3a);
//		printf("peak3: [%i]=>%i\n",i_peak3, peak3);
//		printf("bottom3b: [%i]=>%i\n",i_bottom3b, bottom3b);
		rnet_dry=1000.0;
		dem_dry=2000.0;
		for(row=0;row<nY;row++){
			//NDVI
			GDALRasterIO(hB1,GF_Read,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
			//LAI
			GDALRasterIO(hB2,GF_Read,0,row,nX,1,l2,nX,1,GDT_Float32,0,0);
			//LST
			GDALRasterIO(hB3,GF_Read,0,row,nX,1,l3,nX,1,GDT_Float32,0,0);
			//ALBEDO
			GDALRasterIO(hB4,GF_Read,0,row,nX,1,l4,nX,1,GDT_Float32,0,0);
			//DEM
			GDALRasterIO(hB5,GF_Read,0,row,nX,1,l5,nX,1,GDT_Float32,0,0);
			#pragma omp parallel for default(none) \
				private(col, albedo, t0dem, tempk, dem, e0, lin, lout, lnet, kin, rnet, g_0, h0) \
				shared(nX, row, geomx, tsw, doy, ta, t0dem_min, t0dem_max, tempk_min, tempk_max, \
					tempk_wet, tempk_dry, t0dem_wet, t0dem_dry, \
					g0_wet, g0_dry, rnet_wet, rnet_dry, dem_dry, \
					row_wet, row_dry, col_wet, col_dry, h0_max, \
					i_peak3, i_peak1, h0_min,\
					l1, l2, l3, l4, l5)
			for(col=0;col<nX;col++){
				if(l1[col]==-28768||l3[col]==-28768||l3[col]*0.02>200||l4[col]*0.001>0.001){
				albedo = l4[col]*0.001;
				t0dem = l3[col]*0.02-0.00625*l5[col];
				tempk = l3[col]*0.02;
				dem = l5[col];
				// Surface emissivity calculations (e0=f(ndvi,lai))
				e0=e_0(l1[col]*0.0001,l2[col]*0.1);
// 				printf("e0=%f\n",e0);
				//Incoming longwave radiation (Lin). To be derived locally at CSU (AU)
				tsw = 0.75 + 2 * pow(10,-5) * l5[col];
				//For Sri Lanka
//				tsw = 0.65 + 2 * pow(10,-5) * l5[col];
// 				printf("tsw=%f dem=%f\n", tsw, l5[col]);
				lin = Lin(tsw, ta);
// 				printf("lin=%f\n", lin);
				//Outgoing longwave radiation (Lout=f(e0,lst))
				lout=Lout(e0,tempk);
// 				printf("lout=%f\n",lout);
				//Net longwave radiation (Lnet)
				lnet=Lnet(lout,lin);
// 				printf("lnet=%f\n",lnet);
				//Solar shortwave incoming irradiance
				kin=Kin(doy,geomx[3]+geomx[4]*col+geomx[5]*row,tsw);
// 				printf("lat=%f\n",geomx[3]+geomx[4]*col+geomx[5]*row);
// 				printf("kin=%f\n",kin);
				//Net Radiation (Rnet = f(alb,lnet,kin)
				rnet=Rnet(l4[col]*0.001,lnet,kin);
// 				printf("Alb=%f\n",l4[col]*0.001);
// 				printf("rnet=%f\n",rnet);
				//calculation of g_0 = f(rnet,lst,alb,ndvi)
				g_0=g0(rnet,tempk,l4[col]*0.001,l1[col]*0.0001);
// 				printf("g_0=%f\n",g_0);
				h0 = rnet - g_0;
// 				printf("h0=%f\n",h0);
				if(t0dem>250&&t0dem<t0dem_min&&t0dem>274.0&&h0>0.0&&h0<h0_min&&g_0>0.0){
					t0dem_min=t0dem;
					t0dem_wet=t0dem;
					tempk_min=tempk;
					tempk_wet=tempk;
					rnet_wet=rnet;
					g0_wet=g_0;
					h0_min=h0;
					col_wet=col;
					row_wet=row;
				}
				if(tempk>250&&tempk>=((double)i_peak1-5.0)&&
				tempk<((double)i_peak1+1.0)&&rnet>0.0&&albedo>0.1){
					tempk_min=tempk;
					tempk_wet=tempk;
					t0dem_min=t0dem;
					t0dem_wet=t0dem;
					rnet_wet=rnet;
					g0_wet=g_0;
					h0_min=h0;
					col_wet=col;
					row_wet=row;
				}
				if(t0dem>t0dem_max&&rnet>0.0&&g_0>0.0&&dem<dem_dry&&rnet<rnet_dry){
					t0dem_max=t0dem;
					t0dem_dry=t0dem;
					tempk_max=tempk;
					tempk_dry=tempk;
					rnet_dry=rnet;
					g0_dry=g_0;
					dem_dry=dem;
					col_dry=col;
					row_dry=row;
				}
				if(t0dem>=((double)i_peak3-0.0)&&
				t0dem<((double)i_peak3+7.0)&&
				h0>100.0&&h0>h0_max&&
				g_0>10.0&&rnet<rnet_dry&&
				albedo>0.35&&dem>0.0&&dem<dem_dry){
					tempk_max=tempk;
					tempk_dry=tempk;
					t0dem_max=t0dem;
					t0dem_dry=t0dem;
					rnet_dry=rnet;
					g0_dry=g_0;
					h0_max=h0;
					dem_dry=dem;
					col_dry=col;
					row_dry=row;
				}
				}//END OF pafScanlineOut2 > 0.0
			}
			#pragma omp barrier
		}
		printf("tempk_min=%f\ntempk_max=%f\n",tempk_min,tempk_max);
	}
	printf("row_wet=%d\tcol_wet=%d\n",row_wet,col_wet);
	printf("row_dry=%d\tcol_dry=%d\n\n",row_dry,col_dry);
	printf("t0dem_wet = %f\n",t0dem_wet);
	printf("tempk_wet=%f\n",tempk_wet);
	printf("g0_wet=%f\n",g0_wet);
	printf("rnet_wet=%f\n",rnet_wet);
	printf("LE_wet=%f\n\n",rnet_wet-g0_wet);
	printf("tempk_dry=%f\n",tempk_dry);
	printf("dem_dry=%f\n",dem_dry);
	printf("t0dem_dry=%f\n",t0dem_dry);
	printf("rnet_dry=%f\n",rnet_dry);
	printf("g0_dry=%f\n",g0_dry);

	// calculation of dTair (spreadsheet calculations)
	dTair(a,b,eto_alf,kc,dem_wet,t0dem_wet,rnet_wet,g0_wet,dem_dry,t0dem_dry,rnet_dry,g0_dry);
// 	printf("***OUT***a[0]=%f\tb[0]=%f\n",a[0],b[0]);
// 	PROCESS METRIC
	#pragma omp parallel for default(none) \
		private(rowcol, e0, tsw, lin, lout, lnet, kin, \
		rnet, g_0, z_0m, h0, t0dem, tadem, pmeto, dailyN, \
		metricevapfr, metriceta, metricdtair, metrictheta)\
		shared(N, rh, u, a, b, doy, ta, u200, rah_0, ustar_0, iteration, \
		mat1, mat2, mat3, mat4, mat5, matOut2, matOut1, matOut0, matOut, matLat )
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat1[rowcol]==-28768||mat3[rowcol]==-28768||mat4[rowcol]<=0||
		  mat3[rowcol]*0.02<250.0||mat3[rowcol]*0.02>360.0) {
			matOut[rowcol] = -28768;
			matOut0[rowcol] = -28768;
			matOut1[rowcol] = -28768;
		} else {
			// METRIC ETa (Allen, 2005)
			// Surface emissivity calculations (e0=f(ndvi,lai))
			e0 = e_0(mat1[rowcol]*0.0001,mat2[rowcol]*0.1);
			//Incoming longwave radiation (Lin). To be derived locally at CSU
			//tsw=0.7;//Colombo?
			tsw = 0.75 + 2 * pow(10,-5)*mat5[rowcol];
			lin = Lin(tsw, ta);
			//Outgoing longwave radiation (Lout=f(e0,lst))
			lout = Lout(e0,mat3[rowcol]*0.02);
//  			printf("lout=%f\n",lout);
			//Net longwave radiation (Lnet)
			lnet = Lnet(lout,lin);
//  			printf("lnet=%f\n",lnet);
			//Solar shortwave incoming irradiance
			kin = Kin(doy,matLat[rowcol],tsw);
//  			printf("kin=%f\n",kin);
			//Net Radiation (Rnet = f(alb,lnet,kin)
			rnet = Rnet(mat4[rowcol]*0.001,lnet,kin);
//  			printf("rnet=%f\n",rnet);
			//calculation of g_0 = f(rnet,lst,alb,ndvi)
			g_0 = g0(rnet,mat3[rowcol]*0.02,mat4[rowcol]*0.001,mat1[rowcol]*0.0001);
//  			printf("g_0=%f\n",g_0);
			//Calculation of surface roughness of momentum = f(lai)
			z_0m = z0m(mat2[rowcol]*0.01);
//  			printf("z_0m=%f\n",z_0m);
			//Sensible Heat flux Calculations
//  			printf("a[7]=%f,b[7]=%f,mat3*0.02=%f,rah_0=%f,z_0m=%f,ustar_0=%f,mat5=%f,u200=%f\n",a[7],b[7],mat3[rowcol]*0.02,rah_0,z_0m,ustar_0,mat5[rowcol],u200);
			h0 = metiter(a,b,mat3[rowcol]*0.02,rah_0,z_0m,ustar_0,mat5[rowcol],u200,iteration);
//  			printf("h0=%f\n",h0);
			dailyN = daily_N(doy, matLat[rowcol]);
			t0dem = mat3[rowcol]*0.02-0.00625*mat5[rowcol];
			tadem = t0dem-(a[iteration]*t0dem+b[iteration]);
			//Export dTair to raster file
			metricdtair = (a[iteration]*mat3[rowcol]*0.02)+b[iteration];
			matOut1[rowcol] = metricdtair;
			//ETinst
			metriceta = ETinst(rnet,g_0,h0,tadem);
			matOut[rowcol] = metriceta;
//  			printf("eta=%f\t",metriceta);
			pmeto = EToPM( mat3[rowcol]*0.02, mat5[rowcol], u, rnet*0.0864, rh, 0.6, dailyN);
//  			printf("pmeto=%f\t",pmeto);
			metricevapfr = metriceta / pmeto;
//  			printf("metricevapfr=%f\n",metricevapfr);
			if(metricevapfr < 0.0) matOut0[rowcol] = -28768;
			else matOut0[rowcol] = metricevapfr;
			if(metricevapfr < 0.0) metrictheta = -28768;
			else metrictheta = soilmoisture(metricevapfr);
			matOut2[rowcol] = metrictheta;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut2,GF_Write,0,0,nX,nY,matOut2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut1,GF_Write,0,0,nX,nY,matOut1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut0,GF_Write,0,0,nX,nY,matOut0,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,matOut,nX,nY,GDT_Float32,0,0);
	//free memory close unused files
	if(mat1 != NULL) free(mat1);
	if(mat2 != NULL) free(mat2);
	if(mat3 != NULL) free(mat3);
	if(mat4 != NULL) free(mat4);
	if(mat5 != NULL) free(mat5);
	if(matOut2 != NULL) free(matOut2);
	if(matOut1 != NULL) free(matOut1);
	if(matOut0 != NULL) free(matOut0);
	if(matOut != NULL) free(matOut);
	if(matLat != NULL) free(matLat);

	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hD4);
	GDALClose(hD5);
	GDALClose(hDOut2);
	GDALClose(hDOut1);
	GDALClose(hDOut0);
	GDALClose(hDOut);
	GDALClose(hDLat);
	return(EXIT_SUCCESS);
}

