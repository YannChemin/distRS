/* 2008 November 24
yann Chemin (yann.chemin@gmail.com)
Public domain or LGPL 2/3 or GPL 2/3 as you wish!
Makefile
---------
freq: frequency.c
	gcc -o freq frequency.c -lm -I/usr/include/gdal -L/usr/lib -lgdal
clean:
	rm -f freq
run.sh
---------
root=/home/user/Dir/
files=$(ls $root/*satelliteimages*)
./freq outFREQ $files 
*/
#include<stdio.h>
#include<gdal.h>

#define MAXFILES 15000

void usage()
{
	printf( "-----------------------------------------------------------\n");
	printf( "--Calculate the frequency of occurrence of an event--------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "./freq outFREQ infile1[ infile2 infile3 ...] \n");
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
	int		n_imgs;
	int 	n_null_pix;
	GDALDatasetH hD[MAXFILES+1];
	GDALAllRegister();
	GDALDriverH hDr[MAXFILES+1];
	GDALRasterBandH hB[MAXFILES+1];
	float *l[MAXFILES+1];
	int nX, nY;

	out	 	= argv[1];

	printf("Loading input files:\n");
	n_imgs = argc - 2;
	for (i=0;i<n_imgs;i++){
		printf("%i / %i %s\r",i,n_imgs,argv[i+2]);
		in[i]	= argv[i+2];
		hD[i]	= GDALOpen(in[i],GA_ReadOnly);
		hDr[i]	= GDALGetDatasetDriver(hD[i]);
		hB[i]	= GDALGetRasterBand(hD[i],1);
		nX		= GDALGetRasterBandXSize(hB[0]);
		l[i]	= (float *) malloc(sizeof(float)*nX);
	}
	nY = GDALGetRasterBandYSize(hB[0]);

	//Creating output file
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	hD[n_imgs] = GDALCreateCopy( hDr[0], out,hD[0],FALSE,options,NULL,NULL);
	hB[n_imgs] = GDALGetRasterBand(hD[n_imgs],1);
	l[n_imgs] = (float *) malloc(sizeof(float)*nX);
	
	//Accessing the data rowxrow
	//---------------------------
	for(row=0;row<nY;row++){
		for (i=0;i<n_imgs;i++){
			GDALRasterIO(hB[i],GF_Read,0,row,nX,1,l[i],nX,1,GDT_Float32,0,0);
		}
		//Processing the data cellxcell
		//-----------------------------
		for(col=0;col<nX;col++){
			if(l[i][col] < 0) l[n_imgs][col] = -28768 ;
			else{
				l[n_imgs][col] = 0.0;
				n_null_pix = 0;
				for (i=0;i<n_imgs;i++){
					if(l[i][col] > 1) n_null_pix++;
					else l[n_imgs][col] += l[i][col];
				}
				l[n_imgs][col] /= (n_imgs - n_null_pix);
			}
		}
		for(col=0;col<nX;col++){
				l[n_imgs][col] *= 32000 ;
				/*to recover any positive pixel*/
				//if(l[n_imgs][col]>0&&l[n_imgs][col]<1)
				//	l[n_imgs][col]=1;
		}
		GDALRasterIO(hB[n_imgs],GF_Write,0,row,nX,1,l[n_imgs],nX,1,GDT_Float32,0,0);
	}
	for (i=0;i<n_imgs+1;i++){
		if( l[i] != NULL ) free( l[i] );
		GDALClose(hD[i]);
	}
}

