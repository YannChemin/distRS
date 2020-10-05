#include<stdio.h>
#include "gdal.h"
#include<omp.h>

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./fc inEmis31 inEmis32 outFC\n");
	printf( "-----------------------------------------\n");
	printf( "inEmis31\t\tModis MOD11A2 Emis 31 1Km\n");
	printf( "inEmis32\t\tModis MOD11A2 Emis 32 1Km\n");
	printf( "outFC\t\t\tFraction emissivity Cover output [-x100]\n");
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
	char	*inB2	 	= argv[1]; //Emis31
	char	*inB3	 	= argv[2]; //Emis32
	char	*fcF	 	= argv[3];	
	//Loading the input files
	//-----------------------
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//Emis31
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//Emis32
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	//Loading the file infos
	//----------------------
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,fcF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//Emis31
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//Emis32
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	float tempval, minval=100, maxval=0;
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol, tempval) shared (N, minval, maxval, l2, l3)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l2[rowcol] == 0 || l3[rowcol] == 0) {} 
		else
		{ 
			tempval = 0.49 + 0.001 * (l2[rowcol]+l3[rowcol]);
			if( tempval < minval ) minval = tempval;
			else if ( tempval > maxval && tempval < 1.0 ) maxval = tempval;
		}
	}
	#pragma omp barrier
	printf("\t\tminval=%.3f\tmaxval=%.3f\n",minval,maxval);
	#pragma omp parallel for default(none) \
	private (rowcol, tempval) shared (N, minval, maxval, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l2[rowcol] == 0 || l3[rowcol] == 0){
			lOut[rowcol] = 0;
		} 
		else
		{ 
			tempval = 0.49 + 0.001 * (l2[rowcol]+l3[rowcol]);
			if( tempval < minval ) lOut[rowcol] = 0;
			else if ( tempval > maxval ) lOut[rowcol] = 0;
			else lOut[rowcol] =  100.0 * (float) (tempval-minval)/(maxval-minval);
		}
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

