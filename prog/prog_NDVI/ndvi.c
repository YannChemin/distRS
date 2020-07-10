#include<stdio.h>
#include "gdal.h"
#include<omp.h>
#include "cpl_string.h"

#define NODATA -28768

/*
-1	Fill/No Data	Not Processed
0 	Good Data 	Use with confidence
1 	Marginal data	Useful, but look at other QA information
2 	Snow/Ice 	Target covered with snow/ice
3 	Cloudy 		Target not visible, covered with cloud
*/
void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ndvi inNDVI inNDVI_QA\n");
	printf( "\toutNDVI\n");
	printf( "-----------------------------------------\n");
	printf( "inNDVI\t\tModis MOD13Q1 NDVI 250m\n");
	printf( "inNDVI_QA\t\tModis MOD13Q1 NDVI Reliability\n");

	printf( "outNDVI\tQA corrected NDVI output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return 1;
	}
	char	*inB2	 	= argv[1]; //NDVI
	char	*inB3	 	= argv[2]; //NDVI_QA
	char	*ndviF	 	= argv[3];
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//NDVI
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//NDVI_QA
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,ndviF,hD2,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALSetRasterNoDataValue(hBOut, NODATA);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//NDVI
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//NDVI_QA
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	int err=GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	err=GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol) shared (N, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l3[rowcol] == 0||l3[rowcol] == 1) lOut[rowcol] = l2[rowcol];
		else if(l2[rowcol] == -3000) lOut[rowcol] = NODATA;
		else lOut[rowcol] = NODATA;
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

