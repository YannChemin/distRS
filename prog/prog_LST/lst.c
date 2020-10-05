#include<stdio.h>
#include "gdal.h"
#include "arrays.h"
#include<omp.h>

/* mod11A1 MODLAND_QC bits [00-11]
[00] = class 0 ; LST produced, good quality, not necessary to examine detailed QA
[01] = class 1 ; LST produced, unreliable or unquantifiable quality, recommend examination of more detailed QA
[10] = class 2 : LST not produced due to cloud effects
[11] = class 3 : LST not produced primarily due to reasons other than clouds 
 */

int mod11A1a(int pixel) {
    return (pixel & 0x03);
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./lst inLST inLST_QA\n");
	printf( "\toutLST\n");
	printf( "-----------------------------------------\n");
	printf( "inLST\t\tModis MOD11A1 LST 1000m\n");
	printf( "inLST_QA\t\tModis MOD11A1 LST Reliability\n");

	printf( "outLST\tQA corrected LST output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return (EXIT_FAILURE);
	}
	char	*inB2 	= argv[1]; //LST
	char	*inB3 	= argv[2]; //LST_QA
	char	*lstF	= argv[3];

	//Loading the input files
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//LST
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//LST_QA

	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	//Loading the file infos
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	//Creating output file LST out
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,lstF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);

	//Loading the file bands
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//LST
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//LST_QA

	//Loading the data in RAM
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N=nX*nY;
	float *l2 = af1d(N);
	float *l3 = af1d(N);
	float *lOut = af1d(N);
	int rowcol, qa;

	//LST 1Km
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	//LST_QA 1Km
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private (rowcol, qa) shared (N, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		qa=mod11A1a(l3[rowcol]);
		if( qa == 0 || qa == 1 ) lOut[rowcol] = l2[rowcol];
		else lOut[rowcol] = -28768;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);

	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

