#include<stdio.h>
#include<gdal.h>
// #include"rs.h"

float ndvi(float red, float nir){
	double result;
	if(red+nir<=0.001){
		result=-1.0;
	}else{
		result=(nir-red)/(red+nir);
	}
	return result;
}
int water_modis(double surf_ref_7, double ndvi)
{
	double result;
	if (surf_ref_7<0.04&&ndvi<0.1){
		result = 1 ;
	} else {
		result = 0 ;
	}
	return result;
}

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./wm inB1 inB2 inB7\n");
	printf( "     in250QC in500stateQA\n");
	printf( "     outWATER\n");
	printf( "-----------------------------------------\n");
	printf( "inB[1-7] files are surface reflectance files (250&500)\n");
	printf( "inB1 and inB2 are Modis 250m\n");
	printf( "inB3-7 are Modis 500m, they will be split to 250m\n");
	printf( "in250QC is Modis 250/500m Quality Assessment\n");
	printf( "in500stateQA is Modis 500m State Quality Assessment\n");

	printf(	"outWATER is the Water mask output [0-1]\n");
	return;
}

int qc250a( unsigned int pixel );
int stateqa500a( unsigned int pixel );

int main( int argc, char *argv[] )
{
      	if( argc < 6 ) {
		usage();
		return 1;
	}
	//Loading the input files names
	//-----------------------------
        char	*inB1, *inB2, *inB7;
        char	*in250QC, *in500QC;
	char	*waterF;
	inB1 		= argv[1];
	inB2	 	= argv[2];
	inB7 		= argv[3];

	in250QC		= argv[4];
	in500QC		= argv[5];
	
	waterF		= argv[6];
	printf("inB1=%s\ninB2=%s\ninB7=%s\n",inB1, inB2, inB7);
	//Loading the input files
	//-----------------------
	GDALDatasetH hDataset1;//red
	GDALDatasetH hDataset2;//nir
	GDALDatasetH hDataset7;//swir5
	GDALDatasetH hDataset8;//qc250
	GDALDatasetH hDataset9;//qc500

	GDALAllRegister();

	hDataset1 = GDALOpen(inB1,GA_ReadOnly);
	hDataset2 = GDALOpen(inB2,GA_ReadOnly);
	hDataset7 = GDALOpen(inB7,GA_ReadOnly);
	hDataset8 = GDALOpen(in250QC,GA_ReadOnly);
	hDataset9 = GDALOpen(in500QC,GA_ReadOnly);

	if(hDataset1==NULL||hDataset2==NULL||
	hDataset7==NULL||hDataset8==NULL||
	hDataset9==NULL){
		printf("One or more input files "); 
		printf("could not be loaded\n");
		exit(1);
	}

	//Loading the file infos 
	//----------------------
	GDALDriverH hDriver1;
	GDALDriverH hDriver2;
	GDALDriverH hDriver7;
	GDALDriverH hDriver8;
	GDALDriverH hDriver9;

	hDriver1 = GDALGetDatasetDriver(hDataset1);
	hDriver2 = GDALGetDatasetDriver(hDataset2);
	hDriver7 = GDALGetDatasetDriver(hDataset7);
	hDriver8 = GDALGetDatasetDriver(hDataset8);
	hDriver9 = GDALGetDatasetDriver(hDataset9);

	//Creating output file 
	//--------------------
	//Water Mask out
	GDALDatasetH hDatasetOut0;
	hDatasetOut0 = GDALCreateCopy( hDriver1, waterF,
				hDataset1,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBandOut0;
	hBandOut0 = GDALGetRasterBand(hDatasetOut0,1);

	//Loading the file bands 
	//----------------------
	GDALRasterBandH hBand1;
	GDALRasterBandH hBand2;
	GDALRasterBandH hBand7;
	GDALRasterBandH hBand8;
	GDALRasterBandH hBand9;

	hBand1 = GDALGetRasterBand(hDataset1,1);
	hBand2 = GDALGetRasterBand(hDataset2,1);
	hBand7 = GDALGetRasterBand(hDataset7,1);
	hBand8 = GDALGetRasterBand(hDataset8,1);
	hBand9 = GDALGetRasterBand(hDataset9,1);
	
	printf("Passed 1\n");

	//Loading the data rowxrow
	//------------------------
	float *pafScanline1;
	float *pafScanline2;
	float *pafScanline7;
	unsigned int *pafScanline8; //250m QC
	unsigned int *pafScanline9; // 500m QC
	
	printf("Passed 2\n");
	float *pafScanlineOut0;

	int nXSize1 = GDALGetRasterBandXSize(hBand1);
	int nYSize1 = GDALGetRasterBandYSize(hBand1);

	printf("Passed 3\n");
	pafScanline1 = (float *) CPLMalloc(sizeof(float)*nXSize1);
	pafScanline2 = (float *) CPLMalloc(sizeof(float)*nXSize1);
	pafScanline7 = (float *) CPLMalloc(sizeof(float)*nXSize1/2);
	pafScanline8 = (unsigned int *) CPLMalloc(sizeof(unsigned int)*nXSize1);
	pafScanline9 = (unsigned int *) CPLMalloc(sizeof(unsigned int)*nXSize1/2);
	
	printf("Passed 4\n");
	pafScanlineOut0 = (float *) CPLMalloc(sizeof(float)*nXSize1);

	int row,col;
	
	float ndvix,waterx;	
	float temp, tempval;	
	//Accessing the data rowxrow
	//---------------------------
	for(row=0;row<nYSize1;row++){
		GDALRasterIO(hBand1,GF_Read,0,row,nXSize1,1,
			pafScanline1,nXSize1,1,GDT_Float32,0,0);
		GDALRasterIO(hBand2,GF_Read,0,row,nXSize1,1,
			pafScanline2,nXSize1,1,GDT_Float32,0,0);
		GDALRasterIO(hBand7,GF_Read,0,row/2,nXSize1/2,1,
			pafScanline7,nXSize1/2,1,GDT_Float32,0,0);
		GDALRasterIO(hBand8,GF_Read,0,row,nXSize1,1,
			pafScanline8,nXSize1,1,GDT_UInt32,0,0);
		GDALRasterIO(hBand9,GF_Read,0,row/2,nXSize1/2,1,
			pafScanline9,nXSize1/2,1,GDT_UInt32,0,0);
//		printf("Passed row %i -- 1\n",row);
		//Processing the data cellxcell
		//-----------------------------
		for(col=0;col<nXSize1;col++){
			tempval = stateqa500a(pafScanline9[col/2]);
			temp = qc250a(pafScanline8[col]);
			if(pafScanline1[col]==-28672
			||temp>1.0||tempval>=1.0){
				/*skip it*/
				if(temp>1.0){
					pafScanlineOut0[col]=10.0;
				} else if (tempval>=1.0){
					pafScanlineOut0[col]=100.0;
				} else {
					pafScanlineOut0[col]=-28672;
				}
			} else {
				//NDVI
				ndvix = ndvi(pafScanline1[col],pafScanline2[col]);
				//Water
				waterx = water_modis( 
				pafScanline7[col/2]*0.0001,ndvix);
				pafScanlineOut0[col] = waterx; 
			}
		}
//		printf("Passed row %i -- 2\n",row);
		GDALRasterIO(hBandOut0,GF_Write,0,row,nXSize1,1,
			pafScanlineOut0,nXSize1,1,GDT_Float32,0,0);
	}
	if( pafScanline1 != NULL )
		free( pafScanline1 );
	if( pafScanline2 != NULL )
		free( pafScanline2 );
	if( pafScanline7 != NULL )
		free( pafScanline7 );
	if( pafScanline8 != NULL )
		free( pafScanline8 );
	if( pafScanline9 != NULL )
		free( pafScanline9 );
	if( pafScanlineOut0 != NULL )
		free( pafScanlineOut0 );
	GDALClose(hDataset1);
	GDALClose(hDataset2);
	GDALClose(hDataset7);
	GDALClose(hDataset8);
	GDALClose(hDataset9);
	GDALClose(hDatasetOut0);
}

