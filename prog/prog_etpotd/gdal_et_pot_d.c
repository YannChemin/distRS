
#include<stdio.h>
#include<omp.h>
#include<math.h>
#include "gdal.h"
#include "et_pot_d.h"


void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--OpenMP code----\n");
	printf( "-----------------------------------------\n");
	printf( "./et_pot_d inRnetd inLst\n");
	printf( "\toutEtpotd\n");
	printf( "\t[roh_w]\n");
	printf( "-----------------------------------------\n");

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
        char	*inB1, *inB2;
	char	*et_pot_dF;
	double roh_w=1000.0;
	inB1 		= argv[1]; //Rnetd
	inB2	 	= argv[2]; //LST
	et_pot_dF	= argv[3];
	if(argv[4]) roh_w=atof(argv[4]);

	//Loading the input files
	//-----------------------
	GDALAllRegister();

	GDALDatasetH hD1= GDALOpen(inB1,GA_ReadOnly);//Rnetd
	GDALDatasetH hD2= GDALOpen(inB2,GA_ReadOnly);//LST

	if(hD1==NULL||hD2==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}

	//Loading the file infos
	//----------------------
	GDALDriverH hDr1= GDALGetDatasetDriver(hD1);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	GDALDatasetH hDOut= GDALCreateCopy(hDr1,et_pot_dF,hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut= GDALGetRasterBand(hDOut,1);

	//Loading the file bands
	//----------------------
	GDALRasterBandH hB1= GDALGetRasterBand(hD1,1);//Rnetd
	GDALRasterBandH hB2= GDALGetRasterBand(hD2,1);//LST

	//Loading the data rowxrow
	//------------------------
	int nXSize1 = GDALGetRasterBandXSize(hB1);
	int nYSize1 = GDALGetRasterBandYSize(hB1);
	int N=nXSize1*nYSize1;

	float *mat1 = (float *) malloc(sizeof(float)*N);
	float *mat2 = (float *) malloc(sizeof(float)*N);
	float *matOut = (float *) malloc(sizeof(float)*N);

	float etpotd;
	int rowcol;
	GDALRasterIO(hB1,GF_Read,0,0,nXSize1,nYSize1,mat1,nXSize1,nYSize1,GDT_Float32,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nXSize1,nYSize1,mat2,nXSize1,nYSize1,GDT_Float32,0,0);
	#pragma omp parallel for default(none) \
		private(rowcol, etpotd) \
		shared(N, roh_w, mat1, mat2, matOut)
	for(rowcol=0;rowcol<N;rowcol++){
		if(mat2[rowcol]==-28768||mat2[rowcol]==0) matOut[rowcol] = -28768;
		else {
			etpotd = et_pot_d(mat1[rowcol],mat2[rowcol]*0.02,roh_w);
			matOut[rowcol]=etpotd;
		}
	}
	#pragma omp barrier
	GDALRasterIO(hBOut,GF_Write,0,0,nXSize1,nYSize1,matOut,nXSize1,nYSize1,GDT_Float32,0,0);
	GDALClose(hDOut);
	//free memory close unused files
	if(mat1 != NULL) free(mat1);
	if(mat2 != NULL) free(mat2);
	if(matOut != NULL) free(matOut);

	GDALClose(hD1);
	GDALClose(hD2);
	return(EXIT_SUCCESS);
}

