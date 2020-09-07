#include<stdio.h>
#include<stdlib.h>
#include "gdal.h"
#include<omp.h>
#include "fourier.h"
#include "fillin.h"
#include "movavg.h"

#define OBSERVATION_MAX 46
#define MAXFILES 5000

//NODATA, Top and Bottom Boundary conditions
#define NODATA -32768
#define TBC 10000
#define BBC -10000

#define RC 538
void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./fourier in[in,in,in...]\n");
	printf( "\tout[out,out,out...]\n");
	printf( "-----------------------------------------\n");
	return;
}

// double ndvi_sim[OBSERVATION_MAX] = {0.0};
// double ndvi_obs[OBSERVATION_MAX] = {1.56,1.46,1.51,1.46,1.40,1.41,1.35,1.43,1.33,1.33,1.44,1.44,1.47,1.44,1.60,1.70,1.64,1.65,1.85,2.06,2.14,2.09,2.15,2.20,2.39,2.58,2.36,2.15,2.12,2.63,2.16,2.23,2.24,2.34,2.23,2.11,2.00,1.85,2.05,1.82,1.71,1.76,1.77,1.61,1.69,1.60};

int main( int argc, char *argv[] )
{
	if( argc < 4 ){
		usage();
		return 1;
	}
	//argv[0]="./fourier"
	//argv[last]=(null)
	if((argc-1)%2!=0){
		printf("argv[0]=%s\n",argv[0]);
		printf("argc=%i\n",argc);
		printf("argcm2=%i\n",argc%2);
		printf("input number != output number\n");
		exit(1);
	} 
	char *in,*out;
	int i,j, length = (argc-1)/2;
	int vegetated_seasons=4;
	int imagesperyear=23;//23 for 16 days data, 46 for 8 days data
	int harmonic_number = vegetated_seasons*length/imagesperyear;
	printf("harmonic_number=%i\n",harmonic_number);
	double t_obs[MAXFILES+1]; // temporal signature observed
	double t_sim[MAXFILES+1]; // temporal signature simulated
	double t_fil[MAXFILES+1]; // temporal signature filled in
	double t_avg[MAXFILES+1]; // temporal signature moving averaged
	GDALAllRegister();
	GDALDatasetH hD[MAXFILES+1];
	GDALDatasetH hDOut[MAXFILES+1];
	GDALRasterBandH hB[MAXFILES+1];
	GDALRasterBandH hBOut[MAXFILES+1];
	for(i=0;i<length;i++){
		in=argv[i+1];
// 		printf("i=%i / length = %i\n",i, length);
// 		printf("in=%s\n",in);
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
// 		printf("j=%i / length = %i\n",j, length);
// 		printf("out=%s\n",out);
		hDOut[j] = GDALCreateCopy(hDr,out,hD[0],FALSE,options,NULL,NULL);
		hBOut[j] = GDALGetRasterBand(hDOut[j],1);
	}
	int nX = GDALGetRasterBandXSize(hB[1]);
	int nY = GDALGetRasterBandYSize(hB[1]);
	int N = nX*nY;
	float *l[MAXFILES+1];
	float *lOut[MAXFILES+1];
	int rowcol=N;
	//Load all images into RAM (Careful with that!) 
	for(i=0;i<length;i++){
		lOut[i] = (float *) malloc(sizeof(float)*N);
		for(rowcol=0;rowcol<N;rowcol++){
			lOut[i][rowcol] = 0.0;
		}
		l[i] = (float *) malloc(sizeof(float)*N);
		GDALRasterIO(hB[i],GF_Read,0,0,nX,nY,l[i],nX,nY,GDT_Float32,0,0);
		GDALRasterIO(hBOut[i],GF_Read,0,0,nX,nY,lOut[i],nX,nY,GDT_Float32,0,0);
	}
// 	printf("FILL IN NODATA\n");
// 	PART 1: FILL IN
	int countNODATA=0;
	#pragma omp parallel for default(none) \
		shared(l, lOut, length, harmonic_number, N) \
		private (rowcol,t_obs,t_sim,t_fil,t_avg,i,countNODATA)
	for(rowcol=0;rowcol<N;rowcol++){
// 		printf("%i / %i\n",rowcol, N);
		countNODATA=0;
		for(i=0;i<length;i++){
			t_sim[i]=0.0;
			t_obs[i]=l[i][rowcol];
			if(t_obs[i]>TBC||t_obs[i]<BBC){
				t_obs[i]=NODATA;
				countNODATA++;
			}
		}
		if(rowcol==RC){
			printf("NOData,");
			for(i=0;i<length;i++){
				printf("%f,",t_obs[i]);
			}
			printf("\n");
		}
		if(countNODATA){
// 			printf("rowcol=%i\n",rowcol);
			fillin(t_sim,t_obs,length,NODATA);
			countNODATA=0;
		}
		for(i=0;i<length;i++){
			if(t_obs[i]==NODATA){
				t_fil[i]=t_sim[i];
			} else {
				t_fil[i]=t_obs[i];
			}
		}
		if(rowcol==RC){
			printf("FilledInData,");
			for(i=0;i<length;i++){
				printf("%f,",t_fil[i]);
			}
			printf("\n");
		}
		movavg(t_avg,t_fil,length);
		if(rowcol==RC){
			printf("MovAvgData,");
			for(i=0;i<length;i++){
				printf("%f,",t_avg[i]);
			}
			printf("\n");
		}
		for(i=0;i<length;i++){
			lOut[i][rowcol]=t_avg[i];
		}
	}
	#pragma omp barrier

// 	printf("FOURIER\n");
// 	PART 2: FOURIER
	#pragma omp parallel for default(none) \
		shared(l, lOut, length, harmonic_number, N) \
		private (rowcol,t_obs,t_sim,i)
	for(rowcol=0;rowcol<N;rowcol++){
// 		printf("%i / %i\n",rowcol, N);
		for(i=0;i<length;i++){
			t_sim[i]=0.0;
			t_obs[i]=lOut[i][rowcol];
		}
		fourier(t_sim,t_obs,length,harmonic_number);
		for(i=0;i<length;i++){
			lOut[i][rowcol]=t_sim[i];
		}
		if(rowcol==RC){
			printf("FourierData,");
			for(i=0;i<length;i++){
				printf("%f,",t_sim[i]);
			}
			printf("\n");
		}
	}
	#pragma omp barrier
	for(i=0;i<length;i++){
	  	GDALRasterIO(hBOut[i],GF_Write,0,0,nX,nY,lOut[i],nX,nY,GDT_Float32,0,0);
 		if(l[i]!= NULL) free( l[i] );
 		if(lOut[i]!= NULL) free( lOut[i] );
 		if(hD[i]!=NULL) GDALClose(hD[i]);
 		if(hDOut[i]!=NULL) GDALClose(hDOut[i]);
	}
	return(EXIT_SUCCESS);
}
