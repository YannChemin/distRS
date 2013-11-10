#include <stdio.h>
#include "gdal.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--hist code----\n");
	printf( "-----------------------------------------\n");
	printf( "./hist in nodata > your_text_file.txt\n");
	printf( "-----------------------------------------\n");
	printf( "in\t\timage file to extract hist from\n");
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
	int rc, count = 0;
	float min, max, a, b;
	int temp, histogram[100]={0};
	GDALRasterIO(hB,GF_Read,0,0,nX,nY,l,nX,nY,GDT_Float32,0,0);
	for(rc=0;rc<N;rc++){
		if( l[rc] > 0 && l[rc] != nodata && l[rc] != nodata0 ){
			count++;
			if(min>l[rc]) min=l[rc];
			if(max<l[rc]) max=l[rc];
		}
	}
	a=(100.0-0.0)/(max-min);
	b=0.0-a*min;
	for(rc=0;rc<N;rc++){
		if( l[rc] > 0 && l[rc] != nodata && l[rc] != nodata0){
			temp=(int)( a * l[rc] + b );
			if(temp<=100) histogram[temp]++;
		}
	}
	for(temp=0;temp<100;temp++){
		printf("%7.2f %i\n",(double)temp,histogram[temp]);
	}
	if( l != NULL ) free( l );
	GDALClose(hD);
	return(EXIT_SUCCESS);
}

