#include<stdio.h>
#include "gdal.h"
#include<omp.h>

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ta inFc inETa outTa\n");
	printf( "-----------------------------------------\n");
	printf( "inFc\t\tFraction Veg Cover [0-1] \n");
	printf( "inETa\t\tEvaporative Fraction [0-1]\n");
	printf( "outTa\t\t\tTranspiration Fraction output [0-1]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 3 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
	char	*inB2	 	= argv[1]; //Fc
	char	*inB3	 	= argv[2]; //ETa
	char	*taF	 	= argv[3];	
	//Loading the input files
	//-----------------------
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//Fc
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//ETa
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	//Loading the file infos
	//----------------------
	GDALDriverH hDr3 = GDALGetDatasetDriver(hD3);
	GDALDatasetH hDOut = GDALCreateCopy(hDr3,taF,hD3,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//Fc
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//ETa
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Int32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol) shared (N, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if(l2[rowcol] < 0 || l3[rowcol] < 0) lOut[rowcol] = -28768;
		else
		//FC is in percentage
		lOut[rowcol] = l2[rowcol] * l3[rowcol] / 100.0;
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

