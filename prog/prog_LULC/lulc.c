#include <stdio.h>
#include "gdal.h"
#include <omp.h>

/*
0 	Processed 	Good Quality
1 	Processed	but look at other QA information
2 	Not Processed 	Due to cloud effect
3 	Not Processed	Due to other effects
*/
void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./lulc inLULC inLULC_QA\n");
	printf( "\toutLULC\n");
	printf( "-----------------------------------------\n");
	printf( "inLULC\t\tModis MCD12Q1 LULC 500m\n");
	printf( "inLULC_QA\t\tModis MCD12Q1 LULC QA\n");

	printf( "outLULC\tQA corrected LULC output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return 1;
	}
	char	*inB2	= argv[1]; //LULC
	char	*inB3	= argv[2]; //LULC_QA
	char	*lulcF	= argv[3];
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//LULC
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//LULC_QA
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,lulcF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//LULC
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//LULC_QA
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N=nX*nY;	
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	//LULC 1Km
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	//LULC_QA 1Km
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol) shared (N, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if( (int) l3[rowcol] & 0x00|| (int) l3[rowcol] & 0x01) lOut[rowcol] = l2[rowcol];
		else lOut[rowcol] = -28768;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	if( lOut != NULL ) free( lOut );
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

