#include<stdio.h>
#include<omp.h>
#include<math.h>
#include "gdal.h"
#include "r_net.h"

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./r_net inAlbedo inSunza inEmis31 inEmis32 inLst inDem\n");
	printf( "\toutRNET\n");
	printf( "\tdoy Tmax\n");
	printf( "-----------------------------------------\n");

	printf( "outETPOT\tPotential ET output [mm/d]\n");

	printf( "doy\t\tDay of Year [-]\n");
	printf( "Tmax\t\tMet station Max air temperature [C or K]\n");
	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 9 ) {
		usage();
		return 1;
	}
	char	*inB1 		= argv[1]; //Albedo
	char	*inB2	 	= argv[2]; //Sunza
	char	*inB3		= argv[3]; //e0-1-b31
	char	*inB4		= argv[4]; //e0-2-b32
	char	*inB5		= argv[5]; //LST
	char	*inB6		= argv[6]; //DEM
	char	*rnetF	 	= argv[7];
	float	doy		= atof( argv[8] );
	float	tmax	 	= atof( argv[9] );

	//MDB Farm A
// 	double phase_max=sin(2*3.1415927*(doy+365/3.3)/365);
// 	tmax=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_max);
// // 	double phase_min=sin(2*PI*(doy+365/3.5)/365);
// // 	double tmin=31.17+(36.9-24.1)/2*((1+1/3+1/5+1/7)*phase_min);

	//Convert Tmax from C to K
// 	if(tmax<100.0) tmax+=273.15;

// 	printf("\ndoy\t= %7.2f\ntmax\t= %7.2f\n\n",doy, tmax);
	GDALAllRegister();

	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//Albedo
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//Sunza
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//e31
	GDALDatasetH hD4 = GDALOpen(inB4,GA_ReadOnly);//e32
	GDALDatasetH hD5 = GDALOpen(inB5,GA_ReadOnly);//LST
	GDALDatasetH hD6 = GDALOpen(inB6,GA_ReadOnly);//DEM

	if(hD1==NULL||hD2==NULL||hD3==NULL||
	hD4==NULL||hD5==NULL||hD6==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}

	GDALDriverH hDr6 = GDALGetDatasetDriver(hD6);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr6,rnetF,hD6,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);

	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//Albedo
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//Sunza
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//e31
	GDALRasterBandH hB4 = GDALGetRasterBand(hD4,1);//e32
	GDALRasterBandH hB5 = GDALGetRasterBand(hD5,1);//LST
	GDALRasterBandH hB6 = GDALGetRasterBand(hD6,1);//DEM

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N=nX*nY;

	float *mat1 = (float *) malloc(sizeof(float)*N);
	float *mat2 = (float *) malloc(sizeof(float)*N);
	float *mat3 = (float *) malloc(sizeof(float)*N);
	float *mat4 = (float *) malloc(sizeof(float)*N);
	float *mat5 = (float *) malloc(sizeof(float)*N);
	float *mat6 = (float *) malloc(sizeof(float)*N);
	float *matOut = (float *) malloc(sizeof(float)*N);

	float e0, rnet;
	int rowcol;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,mat1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,mat2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,mat3,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB4,GF_Read,0,0,nX,nY,mat4,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB5,GF_Read,0,0,nX,nY,mat5,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB6,GF_Read,0,0,nX,nY,mat6,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private(rowcol, e0, rnet)\
		shared(N, tmax, doy,\
			mat1,mat2,mat3,mat4,mat5,mat6, \
			matOut )
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat1[rowcol]==-28768||mat5[rowcol]==-28768||mat5[rowcol]==0) matOut[rowcol] = -28768;
		else {
			e0 = 0.5*((mat3[rowcol]*0.002+0.49)+(mat4[rowcol]*0.002+0.49));
			rnet = r_net(mat1[rowcol]*0.001,mat2[rowcol]*0.01,e0,mat5[rowcol]*0.02,mat6[rowcol],doy,tmax);
			matOut[rowcol]=rnet;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,matOut,nX,nY,GDT_Float32,0,0);
	GDALClose(hDOut);
	//free memory close unused files
	if(mat1 != NULL) free(mat1);
	if(mat2 != NULL) free(mat2);
	if(mat3 != NULL) free(mat3);
	if(mat4 != NULL) free(mat4);
	if(mat5 != NULL) free(mat5);
	if(mat6 != NULL) free(mat6);
	if(matOut != NULL) free(matOut);

	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hD4);
	GDALClose(hD5);
	GDALClose(hD6);
	return(EXIT_SUCCESS);
}

