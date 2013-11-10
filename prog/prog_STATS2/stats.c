#include <stdio.h>
#include "gdal.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--stats code----\n");
	printf( "-----------------------------------------\n");
	printf( "./stats in nodata > your_text_file.txt\n");
	printf( "-----------------------------------------\n");
	printf( "in\t\timage file to extract stats from\n");
	printf( "nodata\t\tNODATA value\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 1 ) {
		usage();
		return 1;
	}
	int nodata=-28768;//Standard MODIS NODATA value
	int nodata0 = atoi(argv[2]);
	GDALAllRegister();
	GDALDatasetH hD = GDALOpen(argv[1],GA_ReadOnly);
	if(hD==NULL){
		printf("Input file could not be loaded\n");
		exit(1);
	}
	GDALRasterBandH hB = GDALGetRasterBand(hD,1);
	int nX = GDALGetRasterBandXSize(hB);
	int nY = GDALGetRasterBandYSize(hB);
	int N = nX*nY;
	float *l = (float *) malloc(sizeof(float)*N);
	int rowcol, count = 0;
	float sum = 0.0, min = 100000, max = 0.0, sumdifsqr = 0.0, stdev=0.0;
	GDALRasterIO(hB,GF_Read,0,0,nX,nY,l,nX,nY,GDT_Float32,0,0);
	for(rowcol=0;rowcol<N;rowcol++){
		if( l[rowcol] > 0 && l[rowcol] != nodata && l[rowcol] != nodata0 ){
			count++;
			sum += l[rowcol];
			if(min>l[rowcol]) min=l[rowcol];
			if(max<l[rowcol]) max=l[rowcol];
		}
	}
	float average = sum/(float)count;
	for(rowcol=0;rowcol<N;rowcol++){
		if( l[rowcol] > 0 && l[rowcol] != nodata && l[rowcol] != nodata0 ){
			sumdifsqr += l[rowcol];
		}
	}
	stdev = sqrt(sumdifsqr/count);
	printf("%s,%i,%f,%f,%f,%f,%f\n",argv[1],count,sum,min,max,average,stdev);
	if( l != NULL ) free( l );
	GDALClose(hD);
	return(EXIT_SUCCESS);
}

