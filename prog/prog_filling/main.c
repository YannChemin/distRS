#include<stdio.h>
#include<stdlib.h>
#include "gdal.h"
#include<omp.h>
#include "fillin.h"

#define MAXFILES 7000

//For MODIS NDVI
//#define TBC 10000
//#define BBC -2000
//NODATA, Top and Bottom Boundary conditions
#define NODATA -28768

//For MODIS LST
//#define TBC 20000
//#define BBC 5000
//NODATA, Top and Bottom Boundary conditions
//#define NODATA 0

//For MODIS ALB
//#define TBC 1000
//#define BBC 1
//NODATA, Top and Bottom Boundary conditions
//#define NODATA 0

//For MODIS Ta
#define TBC 14
#define BBC 0

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./filling in[in,in,in...]\n");
	printf( "\tout[out,out,out...]\n");
	printf( "-----------------------------------------\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ){
		usage();
		return 1;
	}
	if((argc-1)%2!=0){
		printf("argv[0]=%s\n",argv[0]);
		printf("argc=%i : %s\n",argc, argv[argc]);
		printf("argcm2=%i\n",(argc-1)%2);
		printf("input number != output number\n");
		exit(1);
	}
	char *in,*out;
	int i,j, length = (argc-1)/2;
	double t_obs[MAXFILES+1]; // temporal signature observed
	double t_sim[MAXFILES+1]; // temporal signature simulated
	GDALAllRegister();
	GDALDatasetH hD[MAXFILES+1];
	GDALDatasetH hDOut[MAXFILES+1];
	GDALRasterBandH hB[MAXFILES+1];
	GDALRasterBandH hBOut[MAXFILES+1];
	for(i=0;i<length;i++){
		in=argv[i+1];
		hD[i] = GDALOpen(in,GA_ReadOnly);
		if(hD[i]==NULL){
			printf("%s could not be loaded\n",in);
			exit(1);
		}
		hB[i] = GDALGetRasterBand(hD[i],1);
	}
	GDALDriverH hDr = GDALGetDatasetDriver(hD[0]);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	for(i=length+1;i<argc;i++){
		j=i-length-1;
		out=argv[i];
		hDOut[j] = GDALCreateCopy(hDr,out,hD[0],FALSE,options,NULL,NULL);
		hBOut[j] = GDALGetRasterBand(hDOut[j],1);
	}
	int nX = GDALGetRasterBandXSize(hB[1]);
	int nY = GDALGetRasterBandYSize(hB[1]);
	int N = nX*nY;
	float *l[MAXFILES+1];
	float *lOut[MAXFILES+1];
	int rc=N;
	//Load all images into RAM (Careful with that!)
	for(i=0;i<length;i++){
		lOut[i] = (float *) malloc(sizeof(float)*N);
		for(rc=0;rc<N;rc++){
			lOut[i][rc] = 0.0;
		}
		l[i] = (float *) malloc(sizeof(float)*N);
		GDALRasterIO(hB[i],GF_Read,0,0,nX,nY,l[i],nX,nY,GDT_Float32,0,0);
		GDALRasterIO(hBOut[i],GF_Read,0,0,nX,nY,lOut[i],nX,nY,GDT_Float32,0,0);
	}
	int countNODATA=0;
	#pragma omp parallel for default(none) \
		shared(l, lOut, length, N) \
		private (rc,t_obs,t_sim,i,countNODATA)
	for(rc=0;rc<N;rc++){
		countNODATA=0;
		for(i=0;i<length;i++){
			t_sim[i]=0.0;
			t_obs[i]=l[i][rc];
			if(t_obs[i]>TBC||t_obs[i]<BBC){
				t_obs[i]=NODATA;
				countNODATA++;
			}
		}
		if(countNODATA!=0&&countNODATA<length){
			fillin(t_sim,t_obs,length,NODATA);
			countNODATA=0;
		}
		for(i=0;i<length;i++){
			if(l[i][rc]>TBC||l[i][rc]<BBC){
				lOut[i][rc]=t_sim[i];
			} else {
				lOut[i][rc]=l[i][rc];
			}
		}
	}
	#pragma omp barrier
	// WRITE OUTPUT
	for(i=0;i<length;i++){
	  	GDALRasterIO(hBOut[i],GF_Write,0,0,nX,nY,lOut[i],nX,nY,GDT_Float32,0,0);
 		if(l[i]!= NULL) free( l[i] );
 		if(lOut[i]!= NULL) free( lOut[i] );
 		if(hD[i]!=NULL) GDALClose(hD[i]);
 		if(hDOut[i]!=NULL) GDALClose(hDOut[i]);
	}
	return(EXIT_SUCCESS);
}
