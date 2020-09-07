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
	GDALAllRegister();

	GDALDatasetH hD1 = GDALOpen(inB1,GA_ReadOnly);//red
	GDALDatasetH hD2 = GDALOpen(inB2,GA_ReadOnly);//nir
	GDALDatasetH hD7 = GDALOpen(inB7,GA_ReadOnly);//swir5
	GDALDatasetH hD8 = GDALOpen(in250QC,GA_ReadOnly);//qc250
	GDALDatasetH hD9 = GDALOpen(in500QC,GA_ReadOnly);//qc500

	if(hD1==NULL||hD2==NULL||hD7==NULL||hD8==NULL||hD9==NULL){
		printf("One or more input files "); 
		printf("could not be loaded\n");
		exit(1);
	}
	//Loading the file infos 
	//----------------------
	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);
	GDALDriverH hDr2 = GDALGetDatasetDriver(hD2);
	GDALDriverH hDr7 = GDALGetDatasetDriver(hD7);
	GDALDriverH hDr8 = GDALGetDatasetDriver(hD8);
	GDALDriverH hDr9 = GDALGetDatasetDriver(hD9);

	//Creating output file 
	//--------------------
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	//Water Mask out
	GDALDatasetH hDOut0 = GDALCreateCopy( hDr1,waterF,hD1,FALSE,options,NULL,NULL);
	GDALDatasetH hBOut0 = GDALGetRasterBand(hDOut0,1);

	//Loading the file bands 
	//----------------------
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);
	GDALRasterBandH hB7 = GDALGetRasterBand(hD7,1);
	GDALRasterBandH hB8 = GDALGetRasterBand(hD8,1);
	GDALRasterBandH hB9 = GDALGetRasterBand(hD9,1);
	
	printf("Passed 1\n");

	//Loading the data rowxrow
	//------------------------
	float *l1;
	float *l2;
	float *l7;
	unsigned int *l8; //250m QC
	unsigned int *l9; // 500m QC
	
	float *lOut0;

	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);

	l1 = (float *) malloc(sizeof(float)*nX);
	l2 = (float *) malloc(sizeof(float)*nX);
	l7 = (float *) malloc(sizeof(float)*nX/2);
	l8 = (unsigned int *) malloc(sizeof(unsigned int)*nX);
	l9 = (unsigned int *) malloc(sizeof(unsigned int)*nX/2);
	
	lOut0 = (float *) malloc(sizeof(float)*nX);

	int row,col;
	
	float ndvix,waterx;	
	float temp, tempval;	
	//Accessing the data rowxrow
	//---------------------------
	for(row=0;row<nY;row++){
		GDALRasterIO(hB1,GF_Read,0,row,nX,1,l1,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB2,GF_Read,0,row,nX,1,l2,nX,1,GDT_Float32,0,0);
		GDALRasterIO(hB7,GF_Read,0,row/2,nX/2,1,l7,nX/2,1,GDT_Float32,0,0);
		GDALRasterIO(hB8,GF_Read,0,row,nX,1,l8,nX,1,GDT_UInt32,0,0);
		GDALRasterIO(hB9,GF_Read,0,row/2,nX/2,1,l9,nX/2,1,GDT_UInt32,0,0);
		//Processing the data cellxcell
		//-----------------------------
		for(col=0;col<nX;col++){
			tempval = stateqa500a(l9[col/2]);
			temp = qc250a(l8[col]);
			if(l1[col]==-28672||temp>1.0||tempval>=1.0){
				/*skip it*/
				if(temp>1.0){
					lOut0[col]=10.0;
				} else if (tempval>=1.0){
					lOut0[col]=100.0;
				} else {
					lOut0[col]=-28672;
				}
			} else {
				//NDVI
				ndvix = ndvi(l1[col],l2[col]);
				//Water
				waterx = water_modis( 
				l7[col/2]*0.0001,ndvix);
				lOut0[col] = waterx; 
			}
		}
//		printf("Passed row %i -- 2\n",row);
		GDALRasterIO(hBOut0,GF_Write,0,row,nX,1,lOut0,nX,1,GDT_Float32,0,0);
	}
	if(l1!=NULL)free(l1);
	if(l2!=NULL)free(l2);
	if(l7!=NULL)free(l7);
	if(l8!=NULL)free(l8);
	if(l9!=NULL)free(l9);
	if(lOut0!=NULL)free(lOut0);
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD7);
	GDALClose(hD8);
	GDALClose(hD9);
	GDALClose(hDOut0);
}

