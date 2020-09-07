#include<stdio.h>
#include "gdal.h"
#include<omp.h>

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ndvi inET inPET inQA\n");
	printf( "\toutEVAPFR\n");
	printf( "-----------------------------------------\n");
	printf( "inET\t\tModis M.D16A1 ET 500m\n");
	printf( "inPET\t\tModis M.D16A1 PET 500m\n");
	printf( "inQA\t\tModis M.D16A2 QA\n");

	printf( "outEVAPFR\tQA corrected EVAPFR output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return 1;
	}
	char	*inB1	 	= argv[1]; //ET
	char	*inB2	 	= argv[2]; //PET
	char	*inB3	 	= argv[3]; //QA
	char	*evapfrF 	= argv[4];
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//ET
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//PET
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//QA
	if(hD1==NULL||hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,evapfrF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//ET
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//PET
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//QA
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l1 = (float *) malloc(sizeof(float)*N);
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,l1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol) shared (N, l1, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l3[rowcol] < 249 
			&& l1[rowcol] < 32761 
			&& l2[rowcol] < 32761 
			&& l1[rowcol] > 0  
			&& l2[rowcol] > 0 ) 
			lOut[rowcol] = l1[rowcol]/l2[rowcol];
		else lOut[rowcol] = -28768;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l1 != NULL ) free( l1 );
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

