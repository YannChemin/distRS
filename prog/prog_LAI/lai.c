#include<stdio.h>
#include "gdal.h"
#include<omp.h>
#include "cpl_string.h"

/* mcd15A3 MODLAND_QC bits [0-1]
 * 0 -> class 0: LAI produced, Good quality (main algorithm with or without saturation)
 * 1 -> class 1: LAI produced, Other Quality (back-up algorithm or fill values)
 */
#define NODATA 255
#define Null 1000000000

int mcd15A3a(int pixel) {
    return (pixel & 0x01);
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./lai inLAI inLAI_QA\n");
	printf( "\toutLAI\n");
	printf( "\t[Offset Scale]\n");
	printf( "-----------------------------------------\n");
	printf( "inLAI\t\tModis MCD15A3 LAI 1000m\n");
	printf( "inLAI_QA\t\tModis MCD15A3 FparLai_QC\n");
	printf( "outLAI\tQA corrected LAI output [-]\n");
	printf( "Offset\t Optional offset (DN2LAI)\n");
	printf( "Scale\t Optional scale (DN2LAI)\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return 1;
	}
	char	*inB2	= argv[1]; //LAI
	char	*inB3 	= argv[2]; //LAI_QA
	char	*laiF 	= argv[3]; // Corrected LAI
	float offset=Null, scale=Null;
	if(argv[4] != NULL && argv[5] != NULL){
		offset 	= atof(argv[4]); // Optional Offset (offset+DN*scale)
		scale 	= atof(argv[5]); // Optional scale (offset+DN*scale)
	}
	GDALAllRegister();
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//LAI
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//LAI_QA
	if(hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	char **options = NULL;
	//options = CSLSetNameValue( options, "TILED", "YES" );
	//options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr2,laiF,hD2,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALSetRasterNoDataValue(hBOut, NODATA);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//LAI
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//LAI_QA

	int nX = GDALGetRasterBandXSize(hB2);
	int nY = GDALGetRasterBandYSize(hB2);
	int N=nX*nY;

	float *l2 = (float *) malloc(sizeof(float)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rc, qa;

	//LAI 1Km
	int err = 0; 
	err=GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	//LAI_QA 1Km
	err=GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private (rc, qa) shared (N, l2, l3, lOut, offset, scale)
	for(rc=0;rc<N;rc++){
		qa=mcd15A3a(l3[rc]);
		if( qa != 0) lOut[rc] = NODATA;
		if(offset!=Null && scale!=Null){
			lOut[rc] = offset + l2[rc] * scale;
		}
		else lOut[rc] = l2[rc];
	}
	#pragma omp barrier
	err=GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	err=err+1;
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

