#include<stdio.h>
#include "gdal.h"
#include<omp.h>
#include "cpl_string.h"

/* mod16A2H MODLAND_QC bits [0-1]
 * 0 -> class 0: ETA produced, Good quality (main algorithm with or without saturation)
 * 1 -> class 1: ETA produced, Other Quality (back-up algorithm or fill values)
 */
#define NODATA 32767
#define Null 1000000000

int mod16A2Ha(int pixel) {
    return (pixel & 0x01);
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./eta inETA inETA_QA\n");
	printf( "\toutETA\n");
	printf( "\t[Offset Scale]\n");
	printf( "-----------------------------------------\n");
	printf( "inETA\t\tModis MOD16A2H ETA 1000m\n");
	printf( "inETA_QA\t\tModis MOD16A2H FparLai_QC\n");
	printf( "outETA\tQA corrected ETA output [-]\n");
	printf( "Offset\t Optional offset (DN2ETA)\n");
	printf( "Scale\t Optional scale (DN2ETA)\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return 1;
	}
	char	*inB2	= argv[1]; //ETA
	char	*inB3 	= argv[2]; //ETA_QA
	char	*etaF 	= argv[3]; // Corrected ETA
	float offset=Null, scale=Null;
	if(argv[4] != NULL && argv[5] != NULL){
		offset 	= atof(argv[4]); // Optional Offset (offset+DN*scale)
		scale 	= atof(argv[5]); // Optional scale (offset+DN*scale)
	}
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//ETA
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//ETA_QA
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,etaF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALSetRasterNoDataValue(hBOut, NODATA);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//ETA
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//ETA_QA

	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N=nX*nY;

	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rc, qa;

	//ETA 1Km
	int err = 0; 
	err=GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	//ETA_QA 1Km
	err=GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private (rc, qa) shared (N, l2, l3, lOut, offset, scale)
	for(rc=0;rc<N;rc++){
		qa=mod16A2Ha(l3[rc]);
		if( qa != 0) lOut[rc] = NODATA;
		if(offset!=Null && scale!=Null){
			lOut[rc] = offset + l2[rc] * scale;
		}
		else lOut[rc] = l2[rc];
	}
	#pragma omp barrier
	err=GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	err=err+1;
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

