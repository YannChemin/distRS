#include<stdio.h>
#include "gdal.h"
#include<omp.h>

/* MODLAND QA Bits 500m long int bits[0-1]
 * 00 -> class 0: Corrected product produced at ideal quality -- all bands
 * 01 -> class 1: Corrected product produced at less than idel quality -- some or all bands
 * 10 -> class 2: Corrected product NOT produced due to cloud effect -- all bands
 * 11 -> class 3: Corrected product NOT produced due to other reasons -- some or all bands mayb be fill value (Note that a value of [11] overrides a value of [01])
 */ 
unsigned int mod09GQa(unsigned int pixel)
{
    /* Select bit 0 and 1 (right-side).
     * hexadecimal "0x03" => binary "11"
     * this will set all other bits to null */
    return (pixel & 0x03);
}
/* Band-wise Data Quality 500m long Int 
 * bits[2-5][6-9][10-13][14-17][18-21][22-25][26-29]
 * 0000 -> class 0: highest quality
 * 0111 -> class 1: noisy detector
 * 1000 -> class 2: dead detector; data interpolated in L1B
 * 1001 -> class 3: solar zenith >= 86 degrees
 * 1010 -> class 4: solar zenith >= 85 and < 86 degrees
 * 1011 -> class 5: missing input
 * 1100 -> class 6: internal constant used in place of climatological data for at least one atmospheric constant
 * 1101 -> class 7: correction out of bounds, pixel constrained to extreme allowable value
 * 1110 -> class 8: L1B data faulty
 * 1111 -> class 9: not processed due to deep ocean or cloud
 * Class 10-15: Combination of bits unused
 */  
unsigned int mod09GQc(unsigned int pixel, int bandno) 
{
    unsigned int qctemp;

    pixel >>= 4 + (4 * (bandno - 1));	/* bitshift [] to [0-3] etc. */
    qctemp = pixel & 0x0F;    
    
    return qctemp;
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ndvi inB1 inB2 inQC inB3\n");
	printf( "\toutNDVI\n");
	printf( "-----------------------------------------\n");
	printf( "inB1/B2\t\tModis MOD09GQ B1 B2 250m\n");
	printf( "inQC\t\tModis MOD09GQ QC 250m\n");
	printf( "inB3\t\tModis MOD09GA B3 QC corrected 250m\n");

	printf( "outNDVI\tQA corrected NDVI 250m output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 5 ) {
		//usage();
		return 1;
	}
	char	*inB1	 	= argv[1]; //B1 250m
	char	*inB2	 	= argv[2]; //B2 250m
	char 	*inQC		= argv[3]; //QC 250m
	char	*inB3	 	= argv[4]; //B3 500m
	char	*ndviF	 	= argv[5];
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//B1
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//B2
	GDALDatasetH hDQC = GDALOpen(inQC,GA_ReadOnly);//QC
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//B3 500m
	if(hD1==NULL||hD2==NULL||hDQC==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr1,ndviF,hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//B1
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//B2
	GDALRasterBandH hBQC = GDALGetRasterBand(hDQC,1);//QC
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//B3 500m
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N = nX*nY;
	float *l1 = (float *) malloc(sizeof(float)*N);
	float *l2 = (float *) malloc(sizeof(float)*N);
	int *lQC = (int *) malloc(sizeof(int)*N);
	float *l3 = (float *) malloc(sizeof(float)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol, qa, qa1, qa2;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,l1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hBQC,GF_Read,0,0,nX,nY,lQC,nX,nY,GDT_Int32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX/2,nY/2,l3,nX,nY,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol,qa,qa1,qa2) shared (N, l1, l2, lQC, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		qa=mod09GQa(lQC[rowcol]);
		qa1=mod09GQc(lQC[rowcol],1);
		qa2=mod09GQc(lQC[rowcol],2);
		if( qa == 0 && qa1 == 0 && qa2 == 0 && l3[rowcol] > 0.18 && (l1[rowcol]+l2[rowcol]) != 0.0) 
			lOut[rowcol] = 1000*(l2[rowcol]-l1[rowcol])/(l2[rowcol]+l1[rowcol]);
		else lOut[rowcol] = -28672;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l1 != NULL ) free( l1 );
	if( l2 != NULL ) free( l2 );
	if( lQC != NULL ) free( lQC );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hDQC);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

