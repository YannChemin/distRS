//Public Domain
//Author: Yann Chemin (2014)
#include <stdio.h>
#include "gdal.h"

//Max number of LU classes in Raster Mask
#define LU_MAX 255

void usage()
{
	printf( "\e[36m-----------------------------------------\n");
	printf( "--Modis Processing chain--stats code----\n");
	printf( "-----------------------------------------\n");
	printf( "./stats in inRastmask nodata > your_text_file.txt\n");
	printf( "-----------------------------------------\n");
	printf( "in\t\timage file to extract stats from\n");
	printf( "inRastmask\timage mask with LU classes\n");
	printf( "nodata\t\tNODATA value\n");
	printf( "-----------------------------------------\n");
	printf( "-OUTPUT Structure------------------------\n");
	printf( "LU_filename,pixcount,sum,min,max,median,pc01,pc05,pc95,pc99,average,stdev\n");
	printf( "-----------------------------------------\e[0m\n");
	return;
}

int conv1D2Drow(int nX, int rowcol){
	return((int) floor((double) rowcol/ (double) nX));
}

int conv1D2Dcol(int row, int rowcol){
	return(rowcol-row);
} 

int conv2D1D(int nX, int col, int row){
	return(row*nX+col);
}

//Multi-scale pixel retrieval 
//both input are flattened from 2D to 1D
//Go back to 2D, reference raster coordinates is used
//then find the target scaled coordinates
//Convert back from 2D to 1D for target
//Return 1D target coordinate
// ref_nX_o_tgt_nX is ratio of nX (Reference LU over target model data)
// ref_nY_o_tgt_nY is ratio of nY (Reference LU over target model data)
int findPixelAcrossScales(int ref_rowcol, int ref_nX, int tgt_nX, int ref_nX_o_tgt_nX, int ref_nY_o_tgt_nY ){
	//Where is LU raster reference pixel?
	//at rowcol
	//convert LU raster reference from 1D to 2D
	int ref_row = conv1D2Drow(ref_nX,ref_rowcol);
	int ref_col = conv1D2Dcol(ref_row,ref_rowcol);
	//convert ref coordinates across scales to target
	int tgt_col = (int) (ref_col/ref_nX_o_tgt_nX);
	int tgt_row = (int) (ref_row/ref_nY_o_tgt_nY);
	//convert 2D to 1D for target raster
	int tgt_rowcol = conv2D1D(tgt_nX,tgt_col,tgt_row);
	//Return the target 1D coordinate cooresponding to ref_rowcol
	return(tgt_rowcol);
}

int main( int argc, char *argv[] )
{
	if( argc < 3 ) {
		usage();
		exit(EXIT_FAILURE);
	}
	int nodata=-28768;//Standard MODIS NODATA value
	int nodata0 = atoi(argv[2]);
	GDALAllRegister();
	GDALDatasetH hD = GDALOpen(argv[1],GA_ReadOnly);
	if(hD==NULL){
		printf("Input file could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	GDALRasterBandH hB = GDALGetRasterBand(hD,1);
	int nX = GDALGetRasterBandXSize(hB);
	int nY = GDALGetRasterBandYSize(hB);
	int N = nX*nY;
	float *l = (float *) malloc(sizeof(float)*N);
	GDALDatasetH hD1 = GDALOpen(argv[2],GA_ReadOnly);
	if(hD1==NULL){
		printf("Input LU Mask could not be loaded\n");
		exit(EXIT_FAILURE);
	}
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	int nX1 = GDALGetRasterBandXSize(hB1);
	int nY1 = GDALGetRasterBandYSize(hB1);
	int N1 = nX1*nY1;
	int rowcol, count = 0;
	float *l1 = (float *) malloc(sizeof(float)*N1);
	float *l2 = (float *) malloc(sizeof(float)*count);/*for median*/
	int i=0,j=0,k=0;/*for LU and median*/
	float temp;/*for median*/
	float average=0.0;
	float sum=0,  min = 100000;
	float max = 0.0, sumdifsqr = 0.0, stdev=0.0;
	float median=0.0, pc01=0.0,pc05=0.0,pc95=0.0,pc99=0.0;
	int stop = 0;/*for LU*/
	GDALRasterIO(hB,GF_Read,0,0,nX,nY,l,nX,nY,GDT_Float32,0,0);
	//LU mask safelly assumed to be Integer 32
	GDALRasterIO(hB1,GF_Read,0,0,nX1,nY1,l1,nX1,nY1,GDT_Int32,0,0);
	//Sample the values in Raster mask, make a LU_array.
	int LU_array[LU_MAX]={0};//More than 255 classes should not happen.
	int tmp_LU;//temporary LU val swap container
	//output LU as an array of values found in input Raster Mask
	for(rowcol=0;rowcol<N;rowcol++){
		i = 0;
		stop = 0;
		while (!stop){
			if ( l1[rowcol] == LU_array[i] ) stop = 1;
			i++;
			if ( i >= LU_MAX ) stop = 2;
		}
		if (stop == 2) {
			//End of array, did not find this new LU, add it.
			LU_array[k]=l1[rowcol];
			k++;	
		}
	}
	//Re-arrange the LU_array in ascending order
	for (i=0;i<LU_MAX;i++){
		for (j=0;j<LU_MAX-1;j++){
			if(LU_array[j]>LU_array[j+1]){
				tmp_LU=LU_array[j];
				LU_array[j]=LU_array[j+1];
				LU_array[j+1]=tmp_LU;
			}
		}
	}
	//output LU_array_count, the count of number of members in LU
	int LU_array_count=0;
	for (i=0;i<LU_MAX;i++){
		if(i!=0 && LU_array[i] !=0){
			LU_array_count++;
		}
	}
	//Make Scaling ratii for bringing input data into LU pixel
	//This assumes that:
	//1 - the input raster Mask and the data have exact same Boundaries/Extents!
	//2 - the LU pixel size is smaller than the data
	double X1_o_X=nX1/nX;//if 1200/400 then = 3
	double Y1_o_Y=nY1/nY;
	//Conversion from data col to mask row => LU[x*3]=data[x]
	
	int LU_idx=0;
	//Loop over the LU_array
	for (LU_idx=0;LU_idx<LU_array_count;LU_idx++){
		j = LU_array[LU_idx];
		//Re-initiliaze variables
		sum = 0.0, min = 100000, max = 0.0, sumdifsqr = 0.0, stdev=0.0;
		temp = 0.0, average = 0.0, count = 0;
		median=0.0, pc01=0.0, pc05=0.0, pc95=0.0, pc99=0.0;
		//Process through scales
		for(rowcol=0;rowcol<N1;rowcol++){
			//row * Ncols + col
			//OneDaarry[row * nXSize + col] 
			//where OneDarray is a float* (to match GDT_Float32)
			//and nXSize = GDALGetRasterBandXSize(hB)
			k = findPixelAcrossScales(rowcol,nX1,nX,X1_o_X,Y1_o_Y);
			if( l[k] > 0 
				&& l[k] != nodata 
				&& l[k] != nodata0 
				&& l1[rowcol] == j ){
				count++;
				sum += l[k];
				if(min>l[k]) min=l[k];
				if(max<l[k]) max=l[k];
			}
		}
		average = sum/(float)count;
		for(rowcol=0;rowcol<N1;rowcol++){
			k = findPixelAcrossScales(rowcol,nX1,nX,X1_o_X,Y1_o_Y);
			if( l[k] > 0 
				&& l[k] != nodata 
				&& l[k] != nodata0 
				&& l1[rowcol] == j ){
				sumdifsqr += l[k];
			}
		}
		stdev = sqrt(sumdifsqr/(float)count);
		/* Median computation */
		count = 0;
		for(rowcol=0;rowcol<N1;rowcol++){
			k = findPixelAcrossScales(rowcol,nX1,nX,X1_o_X,Y1_o_Y);
			if( l[k] > 0 
				&& l[k] != nodata 
				&& l[k] != nodata0 
				&& l1[rowcol] == j ){
				l2[count] = l[k];
				count++;
			}
		}
		for(i=0;i<N1;i++){
			for(rowcol=0;rowcol<count-1;rowcol++){
				if(l2[rowcol] > l2[rowcol+1]){
					temp = l2[rowcol];
					l2[rowcol] = l2[rowcol+1];
					l2[rowcol+1] = temp;		
				}
			}
		}
		median = l2[count/2];
		pc99 = l2[(int)((99/100.0)*count+1/2.0)];
		pc95 = l2[(int)((95/100.0)*count+1/2.0)];
		pc05 = l2[(int)((5/100.0)*count+1/2.0)];
		pc01 = l2[(int)((1/100.0)*count+1/2.0)];
		printf("%i_%s,%i,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",LU_array[LU_idx],argv[1],count,sum,min,max,median,pc01,pc05,pc95,pc99,average,stdev);
	}	

	if( l != NULL ) free( l );
	if( l1 != NULL ) free( l1 );
	if( l2 != NULL ) free( l2 );
	GDALClose(hD);
	GDALClose(hD1);
	return(EXIT_SUCCESS);
}

