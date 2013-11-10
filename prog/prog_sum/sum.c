/* 2009 February 13
yann Chemin (yann.chemin@gmail.com)
Public domain or LGPL 2/3 or GPL 2/3 as you wish!
Makefile
---------
sum: sum.c
	gcc -o sum sum.c -lm -I/usr/include/gdal -L/usr/lib -lgdal1.5.0
run.sh
---------
root=/home/user/Dir/
files=$(ls $root*satelliteimages*)
./sum outSUM $files 
*/
#include<stdio.h>
#include<gdal.h>

#define MAXFILES 5000

void usage()
{
	printf( "-----------------------------------------------------------\n");
	printf( "--Calculate the sum of images--------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "./sum outSUM infile1[ infile2 infile3 ...] \n");
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
	int 	n_null_pix;
	GDALDatasetH hDataset[MAXFILES+1];
	GDALAllRegister();
	GDALDriverH hDriver[MAXFILES+1];
	GDALRasterBandH hBand[MAXFILES+1];
	float *pafScanline[MAXFILES+1];
	int nXSize1, nYSize1;

	out	 	= argv[1];

	//Loading input files 
	n_imgs = argc - 2;
	for (i=0;i<n_imgs;i++){
		printf("%i / %i %s\n",i,n_imgs-1,argv[i+2]);
		in[i]	= argv[i+2];
		hDataset[i] = GDALOpen(in[i],GA_ReadOnly);
		hDriver[i] = GDALGetDatasetDriver(hDataset[i]);
		hBand[i] = GDALGetRasterBand(hDataset[i],1);
		nXSize1 = GDALGetRasterBandXSize(hBand[0]);
		pafScanline[i] = (float *) CPLMalloc(sizeof(float)*nXSize1);
	}
	printf("Passed 1\n");
	nYSize1 = GDALGetRasterBandYSize(hBand[0]);

	//Creating output file 
	hDataset[n_imgs] = GDALCreateCopy( hDriver[0], out,
				hDataset[0],FALSE,NULL,NULL,NULL);
// 	hDataset[n_imgs] = GDALCreate(hDriver[0],out,nXSize1,nYSize1,1,GDT_Float32,NULL);
	hBand[n_imgs] = GDALGetRasterBand(hDataset[n_imgs],1);
	pafScanline[n_imgs] = (float *) CPLMalloc(sizeof(float)*nXSize1);
	
	printf("Passed 2\n");
	//Accessing the data rowxrow
	//---------------------------
	for(row=0;row<nYSize1;row++){
		for (i=0;i<n_imgs;i++){
			GDALRasterIO(hBand[i],GF_Read,0,row,nXSize1,1,
			pafScanline[i],nXSize1,1,GDT_Float32,0,0);
		}
// 		printf("*******\n");
		//Processing the data cellxcell
		//-----------------------------
		for(col=0;col<nXSize1;col++){
// 			printf("Passed 3 %i %i\n",row,col);
			pafScanline[n_imgs][col] = 0;
			n_null_pix = 0;
			for (i=0;i<n_imgs;i++){
				if(pafScanline[i][col] < 0)
					n_null_pix++;
				else
					pafScanline[n_imgs][col] += pafScanline[i][col];
			}
		}
//		printf("Passed row %i -- 2\n",row);
		GDALRasterIO(hBand[n_imgs],GF_Write,0,row,nXSize1,1,
			pafScanline[n_imgs],nXSize1,1,GDT_Float32,0,0);
	}
	for (i=0;i<n_imgs+1;i++){
		if( pafScanline[i] != NULL )
			free( pafScanline[i] );
		GDALClose(hDataset[i]);
	}
}

