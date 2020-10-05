#include<stdio.h>
#include "gdal.h"
#include<omp.h>
#include "cpl_string.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./fc inVI outFC\n");
	printf( "-----------------------------------------\n");
	printf( "inVI\t\tModis Veg Index of your choice\n");
	printf( "outFC\t\t\tFraction Veg Cover output [-x100]\n");
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
	char	*inB2	 	= argv[1]; //VI
	char	*fcF	 	= argv[2];	
	//Loading the input files
	//-----------------------
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//VI
	if(hD2==NULL){
		printf("Input file ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
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
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//VI
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	float tempval, minval=100, maxval=0;
	int err=0; 
	err=GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol, tempval) shared (N, minval, maxval, l2)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l2[rowcol] == -28768) {} 
		else
		{ 
			tempval = 0.0001 * l2[rowcol];
			if( tempval < minval ) minval = tempval;
			else if ( tempval > maxval && tempval < 1.0 ) maxval = tempval;
		}
	}
	#pragma omp barrier
	printf("\t\tminval=%.3f\tmaxval=%.3f\n",minval,maxval);
	#pragma omp parallel for default(none) \
	private (rowcol, tempval) shared (N, minval, maxval, l2, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if( l2[rowcol] == -28768){
			lOut[rowcol] = 0;
		} 
		else
		{ 
			tempval = 0.0001 * l2[rowcol];
			if( tempval < minval ) lOut[rowcol] = 0;
			else if ( tempval > maxval ) lOut[rowcol] = 0;
			else lOut[rowcol] =  100.0 * (float) (tempval-minval)/(maxval-minval);
		}
	}
	#pragma omp barrier
	err=GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l2 != NULL ) free( l2 );
	GDALClose(hD2);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

