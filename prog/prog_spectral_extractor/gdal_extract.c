#include <stdio.h>
#include "gdal.h"

#define PI 3.1415927
#define MAX_FILES 5000
void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./extract inX inY inB [band_number]\n");
	printf( "---------------------------------------------\n");
	printf( "inX inY\t\t pixel coordinates (projected)\n");
	printf( "inB\t\t input imagery\n");
	printf( "band_number\t input imagery band number\n");
	return;
}

int main( int argc, char **argv )
{
      	if( argc < 4 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
	int 	row,col;
	double 	coef, geomx[6]={0.0};

	double projX	= atof(argv[1]);//input X projected
	double projY 	= atof(argv[2]);//input Y projected
	GDALAllRegister();
// 	printf("%s\n",argv[3]);
	GDALDatasetH hD = GDALOpen(argv[3],GA_ReadOnly);
	if(hD==NULL){
		printf("%s could not be loaded\n", argv[3]);
		exit(1);
	}
	GDALRasterBandH hB;
	if(atoi(argv[4])){
		hB = GDALGetRasterBand(hD,atoi(argv[4]));
	} else {
		hB = GDALGetRasterBand(hD,1);
	}
	double *l= (double *) malloc(sizeof(double));
	GDALGetGeoTransform(hD,geomx);
// 	printf( "Origin (ULx,ULy) = (%.6f,%.6f)\n", geomx[0], geomx[3] );
// 	printf( "Pixel Size = (%.6f,%.6f)\n", geomx[1], geomx[5] );
// 	printf( "Rot0 = (%.6f,%.6f)\n", geomx[2], geomx[4] );
	if(geomx[2]<=0.0001||(-geomx[4])<=0.0001){
		col=(projX-geomx[0])/geomx[1];
		row=(geomx[3]-projY)/(-geomx[5]);
// 		printf("col=%i row=%i\n",col,row);
	}else {
		coef = geomx[5]/geomx[4];
		col = (coef*projX-coef*geomx[0]-projY+geomx[3])/(coef*geomx[1]-geomx[2]);
		row = (projX - (geomx[0] + geomx[1] * col))/geomx[2];
// 		printf("col=%i row=%i\n",col,row);
	}
	GDALRasterIO(hB,GF_Read,col,row,1,1,l,1,1,GDT_Float64,0,0);
	printf("%s,%f\n",argv[3],(double) *l);
	GDALClose(hD);
	if( l != NULL ) free( l );
	return(EXIT_SUCCESS);
}

