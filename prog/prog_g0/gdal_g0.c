#include<stdio.h>
#include<omp.h>
#include<math.h>
#include "gdal.h"
#include "g_0.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./g_0 inAlbedo inNdvi inLst inRnet inTime\n");
	printf( "\toutG0\n\t[inRoerink]\n");
	printf( "-----------------------------------------\n");

	printf( "outG0\tSoil Heat Flux output [W/m2]\n");
	printf( "inRoerink\tUse Roerink Hapex-Sahel calibration [0;1]\n");

	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 6 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
	char	*inB1 		= argv[1]; //Albedo
	char	*inB2	 	= argv[2]; //Ndvi
	char	*inB3		= argv[3]; //LST
	char	*inB4		= argv[4]; //Rnet
	char	*inB5		= argv[5]; //Time
	char	*g0F	 	= argv[6];
	int roerink		= atoi( argv[7] );
	//Loading the input files
	//-----------------------
	GDALAllRegister();

	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//Albedo
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//Ndvi
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//LST
	GDALDatasetH hD4 = GDALOpen(inB4,GA_ReadOnly);//Rnet
	GDALDatasetH hD5 = GDALOpen(inB5,GA_ReadOnly);//Time

	if(hD1==NULL||hD2==NULL||hD3==NULL||
	hD4==NULL||hD5==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr4 = GDALGetDatasetDriver(hD4);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy( hDr4, g0F,hD4,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//Albedo
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//NDVI
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//LST
	GDALRasterBandH hB4 = GDALGetRasterBand(hD4,1);//Rnet
	GDALRasterBandH hB5 = GDALGetRasterBand(hD5,1);//Time

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N=nX*nY;

	float *mat1 = (float *) malloc(sizeof(float)*N);
	float *mat2 = (float *) malloc(sizeof(float)*N);
	float *mat3 = (float *) malloc(sizeof(float)*N);
	float *mat4 = (float *) malloc(sizeof(float)*N);
	float *mat5 = (float *) malloc(sizeof(float)*N);
	float *matOut = (float *) malloc(sizeof(float)*N);

	float g0;
	int rowcol;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,mat1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,mat2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,mat3,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB4,GF_Read,0,0,nX,nY,mat4,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB5,GF_Read,0,0,nX,nY,mat5,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private(rowcol, g0)\
		shared(N, roerink,\
			mat1,mat2,mat3,mat4,mat5, \
			matOut )
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat1[rowcol]==-28768||mat2[rowcol]==-28768||mat3[rowcol]==-28768||mat3[rowcol]==0||
		  mat4[rowcol]==-28768||mat5[rowcol]==0) matOut[rowcol] = -28768;
		else {
			g0 = g_0(mat1[rowcol]*0.001,mat2[rowcol]*0.0001,mat3[rowcol]*0.02,mat4[rowcol],mat5[rowcol]*0.1,roerink);
			if(g0<0.0) g0=0.0;
			matOut[rowcol]=g0;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,matOut,nX,nY,GDT_Float32,0,0);
	GDALClose(hDOut);
	//free memory close unused files
	if(mat1 != NULL) free(mat1);
	if(mat2 != NULL) free(mat2);
	if(mat3 != NULL) free(mat3);
	if(mat4 != NULL) free(mat4);
	if(mat5 != NULL) free(mat5);
	if(matOut != NULL) free(matOut);

	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hD4);
	GDALClose(hD5);
	return(EXIT_SUCCESS);
}

