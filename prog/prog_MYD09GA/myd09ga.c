#include<stdio.h>
#include "gdal.h"
#include "arrays.h"
#include<omp.h>

/* MODLAND QA Bits 500m long int bits[0-1]
 * 00 -> class 0: Corrected product produced at ideal quality -- all bands
 * 01 -> class 1: Corrected product produced at less than idel quality -- some or all bands
 * 10 -> class 2: Corrected product NOT produced due to cloud effect -- all bands
 * 11 -> class 3: Corrected product NOT produced due to other reasons -- some or all bands mayb be fill value (Note that a value of [11] overrides a value of [01])
 */ 
unsigned int myd09GAa(unsigned int pixel)
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
unsigned int myd09GAc(unsigned int pixel, int bandno) 
{
    unsigned int qctemp;

    pixel >>= 2 + (4 * (bandno - 1));	/* bitshift [] to [0-3] etc. */
    qctemp = pixel & 0x0F;    
    
    return qctemp;
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./myd09ga inQA inB3\n");
	printf( "\tout\n");
	printf( "-----------------------------------------\n");
	printf( "inQA\t\tModis myd09GA QC_500m_1\n");
	printf( "inB3\t\tModis myd09GA Band3\n");

	printf( "out\tQA corrected B3 output [-]\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 4 ) {
		usage();
		return (EXIT_FAILURE);
	}
	char	*inB 	= argv[1]; //QA
	char	*inB3 	= argv[2]; //B3
	char	*outF	= argv[3];

	//Loading the input files
	GDALAllRegister();
	GDALDatasetH hD = GDALOpen(inB,GA_ReadOnly);//QA
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//B3

	if(hD==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	//Loading the file infos
	GDALDriverH hDr3 = GDALGetDatasetDriver(hD3);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	//Creating output file
	GDALDatasetH hDOut = GDALCreateCopy(hDr3,outF,hD3,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);

	//Loading the file bands
	GDALRasterBandH hB = GDALGetRasterBand(hD,1);//QA
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//B3

	//Loading the data in RAM
	int nX = GDALGetRasterBandXSize(hB3);
	int nY = GDALGetRasterBandYSize(hB3);
	int N=nX*nY;
	unsigned int *l = aui1d(N);
	int *l3 = ai1d(N);
	int *lOut = ai1d(N);
	int rowcol, qa, qa1;

	//myd09GA QA 500m
	GDALRasterIO(hB,GF_Read,0,0,nX,nY,l,nX,nY,GDT_UInt32,0,0);
	//myd09GA B3
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Int32,0,0);
	#pragma omp parallel for default(none) \
		private (rowcol, qa, qa1) shared (N, l, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		qa=myd09GAa(l[rowcol]);
		qa1=myd09GAc(l3[rowcol],3);
		if( qa == 0 || qa1 == 0 ) lOut[rowcol] = l3[rowcol];
		else lOut[rowcol] = -28768;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Int32,0,0);

	if( l != NULL ) free( l );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

