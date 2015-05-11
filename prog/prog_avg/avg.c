/* 2015 January 23
yann Chemin (yann.chemin@gmail.com)
Public domain, unlicensed 
Makefile
---------
avg: avg.c
	gcc -o avg avg.c -lm -I/usr/include/gdal -L/usr/lib -lgdal
run.sh
---------
root=/home/user/Dir/
files=$(ls $root*satelliteimages*)
./avg outAVG $files 
* 
* NULLVAL is < 0
* 
*/
#include<stdio.h>
#include<gdal.h>

#define MAXFILES 5000

void usage()
{
	printf( "-----------------------------------------------------------\n");
	printf( "--Calculate the avg of images--------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "./avg outAVG infile1[ infile2 infile3 ...] \n");
	printf( "-----------------------------------------------------------\n");
	printf( "NULL VAL < 0 \n");
	printf( "-----------------------------------------------------------\n");
	return;
}

int main( int argc, char *argv[] )
{
      	if( argc < 5 ) {
		usage();
		return 1;
	}
	int 	i, row, col;
	char	*in[MAXFILES];
	char	*out;
	int 	imgs_per_year;
	int	n_imgs; 
	int 	n_null_pix; /* Null Pixels counter */
	GDALDatasetH hD[MAXFILES+1];
	GDALAllRegister();
	GDALDriverH hDr[MAXFILES+1];
	GDALRasterBandH hB[MAXFILES+1];
	float *l[MAXFILES+1];
	int nX, nY;

	out 	= argv[1];

	//Loading input files 
	n_imgs = argc - 2;
	for (i=0;i<n_imgs;i++){
		printf("%i / %i %s\n",i,n_imgs-1,argv[i+2]);
		in[i]	= argv[i+2];
		hD[i] = GDALOpen(in[i],GA_ReadOnly);
		hDr[i] = GDALGetDatasetDriver(hD[i]);
		hB[i] = GDALGetRasterBand(hD[i],1);
		nX = GDALGetRasterBandXSize(hB[0]);
		l[i] = (float *) malloc(sizeof(float)*nX);
	}
	nY = GDALGetRasterBandYSize(hB[0]);
	//Creating output file 
	hD[n_imgs]=GDALCreateCopy(hDr[0],out,hD[0],FALSE,NULL,NULL,NULL);
// 	hD[n_imgs] = GDALCreate(hDr[0],out,nX,nY,1,GDT_Float32,NULL);
	hB[n_imgs] = GDALGetRasterBand(hD[n_imgs],1);
	l[n_imgs] = (float *) malloc(sizeof(float)*nX);
	//Accessing the data rowxrow
	//---------------------------
	for(row=0;row<nY;row++){
		for (i=0;i<n_imgs;i++){
			GDALRasterIO(hB[i],GF_Read,0,row,nX,1,l[i],nX,1,GDT_Float32,0,0);
		}
// 		printf("*******\n");
		//Processing the data cellxcell
		//-----------------------------
		for(col=0;col<nX;col++){
			l[n_imgs][col] = 0;
			n_null_pix = 0;
			for (i=0;i<n_imgs;i++){
				if(l[i][col] < 0)
					n_null_pix++;
				else
					l[n_imgs][col] += l[i][col];
			}
			l[n_imgs][col] /= (nX-n_null_pix);
		}
//		printf("Passed row %i -- 2\n",row);
		GDALRasterIO(hB[n_imgs],GF_Write,0,row,nX,1,
			l[n_imgs],nX,1,GDT_Float32,0,0);
	}
	for (i=0;i<n_imgs+1;i++){
		if(l[i]!=NULL) free(l[i]);
		GDALClose(hD[i]);
	}
	return(EXIT_SUCCESS);
}

