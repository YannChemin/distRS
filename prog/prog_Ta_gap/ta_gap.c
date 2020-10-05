#include<stdio.h>
#include<gdal.h>
#include<omp.h>

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ta_gap inETpotd inETa inFC outTa_gap\n");
	printf( "-----------------------------------------\n");
	printf( "inETpotd\tET Potential Diurnal (mm/d) \n");
	printf( "inETa\t\tET actual (mm/d) \n");
	printf( "inFC\t\tFraction of Vegetation Cover [0-1]\n");
	printf( "outTa\t\tTranspiration Gap output (mm/d)\n");
	return;
}

int main( int argc, char *argv[] )
{
	if( argc < 3 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
	char	*inB1	 	= argv[1]; //ETpotd
	char	*inB2	 	= argv[2]; //ETa
	char	*inB3	 	= argv[3]; //FC
	char	*taF	 	= argv[4]; //Ta Gap Outfile
	//Loading the input files
	//-----------------------
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//ETpotd
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//ETa
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//FC
	if(hD1==NULL||hD2==NULL||hD3==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	//Loading the file infos
	//----------------------
	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	GDALDatasetH hDOut = GDALCreateCopy(hDr1,taF,hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);//ETpotd
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);//ETa
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//FC
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);
	int N = nX*nY;
	float *l1 = (float *) malloc(sizeof(float)*N);
	float *l2 = (float *) malloc(sizeof(float)*N);
	short int *l3 = (short int *) malloc(sizeof(short int)*N);
	float *lOut = (float *) malloc(sizeof(float)*N);
	int rowcol;
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,l1,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,l2,nX,nY,GDT_Float32,0,0);
	GDALRasterIO(hB3,GF_Read,0,0,nX,nY,l3,nX,nY,GDT_Int16,0,0);
	#pragma omp parallel for default(none) \
	private (rowcol) shared (N, l1, l2, l3, lOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if(l1[rowcol] < 0 || l2[rowcol] < 0
		|| l3[rowcol] < 0) lOut[rowcol] = -28768;
		else
		//FC is in percentage
		lOut[rowcol] = (l1[rowcol] - l2[rowcol]) * l3[rowcol] / 100.0;
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	if( l1 != NULL ) free( l1 );
	if( l2 != NULL ) free( l2 );
	if( l3 != NULL ) free( l3 );
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

