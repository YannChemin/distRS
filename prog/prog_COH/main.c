#include<stdio.h>
#include "gdal.h"
#include<omp.h>

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./coh inCOH inCOH_QA\n");
	printf( "\toutCOH\n");
	printf( "-----------------------------------------\n");
	printf( "inB2\t\tSentinel 1 Master\n");
	printf( "inB3\t\tSentinel 1 Slave\n");
	printf( "outCOH\tCoherence output [-]\n");
	printf( "windowSize\tsize of window used (i.e. 2 or 3) [-]\n");
	return;
}

float coherenceWindowed(int len, float * data1, float * data2);

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return(EXIT_FAILURE);
	}
	char	*inB2	 	= argv[1]; //COH Master
	char	*inB3	 	= argv[2]; //COH Slave
	char	*cohF	 	= argv[3]; //COH output
	int		nwin		= atoi(argv[4]); //window size
	int		n;
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//COH Master
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//COH Slave
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,cohF,hD2,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//COH Master
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//COH Slave
	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N = nX*nY;
	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);

	int rowcol;
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	//Prepare moving window buffers
	float *data1 = (float *) malloc(sizeof(float)*nwin);
	float *data2 = (float *) malloc(sizeof(float)*nwin);
	for (n=0;n<nwin;n++){ 
		data1[n]=0.0;
		data2[n]=0.0;
	}
	#pragma omp parallel for default(none) \
	private (rowcol,data1,data2,n) shared (N, l2, l3, lOut, nwin)
	for(rowcol=0;rowcol<N;rowcol++){
		lOut[rowcol]=0.0;
		for (n=0;n<nwin;n++){ 
			data1[n]=0.0;
			data2[n]=0.0;
		}
		if(rowcol % nwin == 0){ 
			for (n=0;n<nwin;n++){ 
				data1[n]=l2[rowcol+n];
				data2[n]=l3[rowcol+n];
			}
			lOut[rowcol] = coherenceWindowed(n,data1,data2);
			for (n=0;n<nwin;n++){
				lOut[rowcol+n]=lOut[rowcol];
			}	
		} else {
			lOut[rowcol] = -28768;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	if( data1 != NULL ) free( data1 );
	if( data2 != NULL ) free( data2 );
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

