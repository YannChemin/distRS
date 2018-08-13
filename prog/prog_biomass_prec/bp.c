#include<stdio.h>
#include "gdal.h"
#include<omp.h>

#define NODATA 28768

void usage()
{
	printf( "-----------------------------------------\n");
	printf( "--Modis Processing chain--Serial code----\n");
	printf( "-----------------------------------------\n");
	printf( "./ndvi inNDVI inET inPET\n");
	printf( "\toutNDVI\n");
	printf( "\tDOY Tsw\n");
	printf( "-----------------------------------------\n");
	printf( "inNDVI\t\tModis MOD13Q1 NDVI 250m\n");
	printf( "inET\t\tModis MOD16A2 ET 500m\n");
	printf( "inPET\t\tModis MOD16A2 PET 500m\n");

	printf( "outNDVI\tQA corrected NDVI output [-]\n");

	printf( "DOY\tDay of Year\n");
	printf( "Tsw\tTransmissivity single-way [-]\n");
	return;
}

double biomass(double fpar, double solar_day, double evap_fr, double light_use_ef);
double solar_day(double lat, double doy, double tsw);

int main( int argc, char *argv[] )
{
	if( argc < 6 ) {
		usage();
		return 1;
	}
	char	*inB3	 	= argv[1]; //NDVI_QA 250m
	char	*inB4	 	= argv[2]; // ET 500m
	char	*inB5	 	= argv[3]; // PET 500m
	char	*bpF	 	= argv[4];
	int	doy		= atoi(argv[5]); // DOY for solar day
	float	tsw		= atof(argv[6]); // TSW for solar day

	GDALAllRegister();
	GDALDatasetH hD3 = GDALOpen(inB3,GA_ReadOnly);//NDVI 250m
	GDALDatasetH hD4 = GDALOpen(inB4,GA_ReadOnly);//ET 500m
	GDALDatasetH hD5 = GDALOpen(inB5,GA_ReadOnly);//PET 500m
	if(hD3==NULL||hD4==NULL||hD5==NULL){
		printf("One or more input files ");
		printf("could not be loaded\n");
		exit(1);
	}
	//LOAD 250 m bands and create output
	GDALDriverH hDr3 = GDALGetDatasetDriver(hD3);
	GDALDatasetH hDOut = GDALCreateCopy(hDr3,bpF,hD3,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);//NDVI 250m
	GDALRasterBandH hB4 = GDALGetRasterBand(hD4,1);//ET 500m
	GDALRasterBandH hB5 = GDALGetRasterBand(hD5,1);//PET 500m
	double 	geomx[6]={0.0};
	if(GDALGetGeoTransform(hD3,geomx)==CE_None){
	/* Do Nothing */
	//printf( "Origin (ULx,ULy) = (%.6f,%.6f)\n", geomx[0], geomx[3] );
	//printf( "Pixel Size = (%.6f,%.6f)\n", geomx[1], geomx[5] );
	//printf( "Rot0 = (%.6f,%.6f)\n", geomx[2], geomx[4] );
	} else {
		printf("ERROR: Projection acquisition problem from Band1\n");
		exit(1);
	}
	int nX = GDALGetRasterBandXSize(hB3);//From NDVI@250m
	int nY = GDALGetRasterBandYSize(hB3);//From NDVI@250m
	int16_t *l3 = (int16_t *) malloc(sizeof(int16_t)*nX);
	int16_t *l4 = (int16_t *) malloc(sizeof(int16_t)*nX/2);
	int16_t *l5 = (int16_t *) malloc(sizeof(int16_t)*nX/2);
	unsigned int *lOut = (unsigned int *) malloc(sizeof(unsigned int) *nX);
	int row,col;
	int minimum=100000.0;
	int maximum = 0.0;
	for(row=0;row<nY;row++){
		GDALRasterIO(hB3,GF_Read,0,row,nX,1,l3,nX,1,GDT_Int16,0,0);
		GDALRasterIO(hB4,GF_Read,0,row/2,nX/2,1,l4,nX/2,1,GDT_Int16,0,0);
		GDALRasterIO(hB5,GF_Read,0,row/2,nX/2,1,l5,nX/2,1,GDT_Int16,0,0);
		#pragma omp parallel for default(none) \
			private (col) \
			shared (row,geomx,doy,tsw,nX,nY,l3,l4,l5,lOut,minimum,maximum)
		for(col=0;col<nX;col++){
			if(l4[col/2]<32000||l5[col/2]<32000){ 
				double lat=geomx[3]+geomx[4]*col+geomx[5]*row;
				double solar = solar_day(lat, doy, tsw );
				if(solar<0.0) solar=0.0;
				double evapfr = l4[col/2]/(1.0*l5[col/2]); 
				if(evapfr<0.0) evapfr=0.0;
				double fpar=0.0;
				fpar = 1.257*(l3[col]/10000.0)-0.161;
				if(fpar<0.0) fpar=0.0;
				lOut[col]=(unsigned int) 10000.0*biomass(fpar,solar,evapfr,1.0);
				if(lOut[col]<minimum)minimum=lOut[col];
				if(lOut[col]>maximum)maximum=lOut[col];
			}else{
				lOut[col] = NODATA;
			}
		}
		#pragma omp barrier
		GDALRasterIO(hBOut,GF_Write,0,row,nX,1,lOut,nX,1,GDT_UInt32,0,0);
	}
	printf("min/max: %f %f [kg/ha/day]\n",minimum/10000.0,maximum/10000.0);
	GDALSetRasterNoDataValue(hBOut,NODATA);
	GDALSetRasterUnitType(hBOut,"Biomass [kg/ha/day]");
	GDALSetDescription(hBOut,"250m 8 Days mean Biomass");
	GDALSetMetadataItem(hDOut,"*","",NULL);
	if(l3 != NULL) free(l3);
	if(l4 != NULL) free(l4);
	if(l5 != NULL) free(l5);
	GDALClose(hD3);
	GDALClose(hD4);
	GDALClose(hD5);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}

