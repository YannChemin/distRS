// Makefile
// ---------
// alb: alb.c
// 	gcc -o alb alb.c -lm -I/usr/include/gdal -L/usr/lib -lgdal1.5.0
// run.sh
// ---------
#include<stdio.h>
#include<omp.h>
#include<math.h>
#include"gdal.h"

void usage()
{
	printf( "-----------------------------------------------------------\n");
	printf( "--Calculate the weighted sum of WSA/BSA Albedo images------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "--DFS: Diffuse Fraction of Sunlight------------------------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "./alb outALB DFS inWSAfile inBSAfile inQA\n");
	printf( "-----------------------------------------------------------\n");
	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 5 ) {
		usage();
		return 1;
	}
	char	*out	= argv[1];
	float	dfs	= atof(argv[2]);
	char 	*in1	= argv[3]; /*WSA*/
	char 	*in2	= argv[4]; /*BSA*/
	char 	*in3	= argv[5]; /*QA*/
	//printf("DFS = %f\n",dfs);
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(in1,GA_ReadOnly);
	GDALDatasetH hD2 = GDALOpen(in2,GA_ReadOnly);
	GDALDatasetH hD3 = GDALOpen(in3,GA_ReadOnly);

	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);

	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);

	int	rowcol;
	int	N = nX*nY;

	float	*l1 = (float *) malloc(sizeof(float)*N);
	float	*l2 = (float *) malloc(sizeof(float)*N);
	float	*l3 = (float *) malloc(sizeof(float)*N);

	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr1,out,hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	float	*lOut = (float *) malloc(sizeof(float)*N);

	/*The Albedo is calculated from BSA, WSA, dfs and QA*/
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,l1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Float32,0,0);

	/*Histogram seek of peaks and bottoms, then stretch to standard Water and Desert Albedo values*/
	int i,temp, histogram[100]={0};
	double a, b; //slope and offset for histogram stretch

	#pragma omp parallel for default(none) \
		private (rowcol,temp) \
		shared (N, dfs, l1, l2, l3, lOut,histogram)
	for(rowcol=0;rowcol<N;rowcol++){
		if(l3[rowcol]==0.0||l3[rowcol]==1.0){
			//[0-1000]->[0-100]
			lOut[rowcol] = (l1[rowcol] * dfs + (1-dfs)*l2[rowcol])*0.1;
			temp=(int)lOut[rowcol];
			if(temp<=100) histogram[temp]++;
			else lOut[rowcol]=-28768;
		} else {
			lOut[rowcol] = -28768;
		}
	}
	#pragma omp barrier
// 	printf("\nHistogram of Albedo\n");
// 	for(temp=0;temp<100;temp++){
// 		printf("histogram[%f]=%i\n",(double)temp*0.01,histogram[temp]);
// 	}
	int peak1, peak2, peak3;
	int i_peak1, i_peak2, i_peak3;
	peak1=0;
	peak2=0;
	peak3=0;
	i_peak1=0;
	i_peak2=0;
	i_peak3=0;
	for(i=0;i<100;i++){
		/*Search for peaks of datasets (1)*/
		if(i<=10){
			if(histogram[i]>peak1){
				peak1 = histogram[i];
				i_peak1=i;
			}
		}
		/*Search for peaks of datasets (2)*/
		if(i>=10&&i<=30){
			if(histogram[i]>peak2){
				peak2 = histogram[i];
				i_peak2=i;
			}
		}
		/*Search for peaks of datasets (3)*/
		if(i>=30){
			if(histogram[i]>peak3){
				peak3 = histogram[i];
				i_peak3=i;
			}
		}
	}
	int bottom1a, bottom1b;
	int bottom2a, bottom2b;
	int bottom3a, bottom3b;
	int i_bottom1a, i_bottom1b;
 	int i_bottom2a, i_bottom2b;
 	int i_bottom3a, i_bottom3b;
	bottom1a=100000;
	bottom1b=100000;
	bottom2a=100000;
	bottom2b=100000;
	bottom3a=100000;
	bottom3b=100000;
	i_bottom1a=100;
	i_bottom1b=100;
	i_bottom2a=100;
	i_bottom2b=100;
	i_bottom3a=100;
	i_bottom3b=100;
	/* Water histogram lower bound */
	for(i=0;i<i_peak1;i++){
		if(histogram[i]<=bottom1a){
			bottom1a = histogram[i];
			i_bottom1a = i;
		}
	}
	/* Water histogram higher bound */
	for(i=i_peak2;i>i_peak1;i--){
		if(histogram[i]<=bottom1b){
			bottom1b = histogram[i];
			i_bottom1b = i;
		}
	}
	/* Land histogram lower bound */
	for(i=i_peak1;i<i_peak2;i++){
		if(histogram[i]<=bottom2a){
			bottom2a = histogram[i];
			i_bottom2a = i;
		}
	}
	/* Land histogram higher bound */
	for(i=i_peak3;i>i_peak2;i--){
		if(histogram[i]<bottom2b){
			bottom2b = histogram[i];
			i_bottom2b = i;
		}
	}
	/* Cloud/Snow histogram lower bound */
	for(i=i_peak2;i<i_peak3;i++){
		if(histogram[i]<bottom3a){
			bottom3a = histogram[i];
			i_bottom3a = i;
		}
	}
	/* Cloud/Snow histogram higher bound */
	for(i=100;i>i_peak3;i--){
		if(histogram[i]<bottom3b){
			bottom3b = histogram[i];
			i_bottom3b = i;
		}
	}
	//printf("peak1 %d %d\n",peak1, i_peak1);
	//printf("bottom1a %d %d\n",bottom1a, i_bottom1a);
	//printf("bottom2b= %d %d\n",bottom2b, i_bottom2b);
	if(i_bottom2b==100||i_bottom1a==100){
		a=1;
		b=0;
		printf("a= %f\tb= %f\n",a,b);
		#pragma omp parallel for default(none) \
			private (rowcol) \
			shared (N, lOut, a, b)
		for(rowcol=0;rowcol<N;rowcol++)
			if(lOut[rowcol] > 0) lOut[rowcol] = 10 * lOut[rowcol];
		#pragma omp barrier
	} else {
		a=(0.36-0.05)/(i_bottom2b/100.0-i_bottom1a/100.0);
		b=0.05-a*(i_bottom1a/100.0);
		printf("a= %f\tb= %f\n",a,b);
		#pragma omp parallel for default(none) \
			private (rowcol) \
			shared (N, lOut, a, b)
		for(rowcol=0;rowcol<N;rowcol++)
			if(lOut[rowcol] > 0) lOut[rowcol] = 10 * (a * lOut[rowcol] + b);
		#pragma omp barrier
	}
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if(l1 != NULL) free(l1);
	if(l2 != NULL) free(l2);
	if(l3 != NULL) free(l3);
	if(lOut != NULL) free(lOut);
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}
