#include<stdio.h>
#include<omp.h>
#include<math.h>
#include "gdal.h"
#include "sseb_eta.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./sseb_eta inLst inEt0pm\n");
	printf( "\toutSseb_evapfr outSseb_eta outSseb_theta\n");
	printf( "-----------------------------------------\n");

	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 5 ) {
		usage();
		return 1;
	}
	char	*inB1 		= argv[1]; //LST
	char	*inB2	 	= argv[2]; //ET0PM
	char	*sseb_evapfrF	= argv[3];
	char	*sseb_etaF	= argv[4];
	char	*sseb_thetaF	= argv[5];//soil moisture

	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//LST
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//ET0PM
	if(hD1==NULL||hD2==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	//theta out
	GDALDatasetH hDOut1 = GDALCreateCopy( hDr2, sseb_thetaF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut1 = GDALGetRasterBand(hDOut1,1);
	//Evapfr out
	GDALDatasetH hDOut0 = GDALCreateCopy( hDr2, sseb_evapfrF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut0 = GDALGetRasterBand(hDOut0,1);
	//ETa out
	GDALDatasetH hDOut = GDALCreateCopy( hDr2, sseb_etaF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//LST
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//ET0PM
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N=nX*nY;

	float *mat1	= (float *) malloc(sizeof(float)*N);
	float *mat2	= (float *) malloc(sizeof(float)*N);
	float *matOut1	= (float *) malloc(sizeof(float)*N);
	float *matOut0	= (float *) malloc(sizeof(float)*N);
	float *matOut	= (float *) malloc(sizeof(float)*N);

	float ssebevapfr, ssebeta, ssebtheta;
	float lst_h = 0.0;
	float lst_c = 400.0;
	int n;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,mat1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,mat2,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private(n)\
		shared(N, mat1, lst_h, lst_c )
	for(n=0;n<N;n++){
		if(mat1[n]*0.02>250.0&&mat1[n]*0.02<345.0){
			if (mat1[n]*0.02>lst_h) lst_h=mat1[n]*0.02;
			if (mat1[n]*0.02<lst_c) lst_c=mat1[n]*0.02;
		}
	}
	#pragma omp barrier
	printf("cold=%f\thot=%f\n",lst_c,lst_h);
	#pragma omp parallel for default(none) \
		private(n, ssebeta, ssebevapfr, ssebtheta)\
		shared(N, lst_h, lst_c, mat1, mat2, matOut1, matOut0, matOut )
	for(n=0;n<N;n++){
		if(mat1[n]==-28768||mat1[n]==0){
			matOut[n] = -28768;
			matOut0[n] = -28768;
		} else {
			ssebevapfr = sseb_evapfr(lst_h,lst_c,mat1[n]*0.02);
			matOut0[n] = ssebevapfr;
			ssebeta = ssebevapfr * mat2[n];
			matOut[n] = ssebeta;
			if(ssebevapfr>=1) ssebtheta = soilmoisture(1.0);
			else ssebtheta = soilmoisture(ssebevapfr);
			matOut1[n] = ssebtheta;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut1,GF_Write,0,0,nX,nY,matOut1,nX,nY,GDT_Float32,0,0);
	GDALClose(hDOut1);
	GDALRasterIO(hBOut0,GF_Write,0,0,nX,nY,matOut0,nX,nY,GDT_Float32,0,0);
	GDALClose(hDOut0);
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,matOut,nX,nY,GDT_Float32,0,0);
	GDALClose(hDOut);
	if(mat1 != NULL) free(mat1);
	if(mat2 != NULL) free(mat2);
	if(matOut1 != NULL) free(matOut1);
	if(matOut0 != NULL) free(matOut0);
	if(matOut != NULL) free(matOut);
	GDALClose(hD1);
	GDALClose(hD2);
	return(EXIT_SUCCESS);
}

