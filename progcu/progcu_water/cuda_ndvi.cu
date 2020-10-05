/*###############################################################################
#
# Project:  Open Source RS Programming Starter Book
# Purpose:  Satellite image processing functions examples
# Author:   Yann Chemin, <yann.chemin@gmail.com>
#
###############################################################################
# Copyright (c) 2008, Yann Chemin <yann.chemin@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
###############################################################################*/
#include <stdio.h>
#include <gdal.h>
#include <stdlib.h>

__global__ void waterid(float *red, float *nir, float *b7, float *water, int nXS)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x ;
	if ( i < nXS ){
		water[i]=100.f*(1.f+((nir[i]-red[i])/(red[i]+nir[i])));
		if(water[i] < 0.1 && b7[i] < 0.04)
			water[i] = 1;
		else
			water[i] = 0;	
	}
}

int main()
{
	/**GDAL STUFF***************/
	int err; /*catch errors*/
	//Loading the input files
	GDALAllRegister();
	GDALDatasetH hD1 = GDALOpen("b1.tif",GA_ReadOnly);
	GDALDatasetH hD2 = GDALOpen("b2.tif",GA_ReadOnly);
	GDALDatasetH hD3 = GDALOpen("b7.tif",GA_ReadOnly);

	if(hD1==NULL||hD2==NULL||hD3==NULL){
		printf("At least one input file could not be loaded\n");
		exit(EXIT_FAILURE);
	}

	//Loading the file infos 
	GDALDriverH hDr1 = GDALGetDatasetDriver(hD1);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	//Creating output file
	GDALDatasetH hDOut = GDALCreateCopy( hDr1, "water.tif",hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut = GDALGetRasterBand(hDOut,1);
	
	//Loading the file bands 
	GDALRasterBandH hB1 = GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 = GDALGetRasterBand(hD2,1);
	GDALRasterBandH hB3 = GDALGetRasterBand(hD3,1);
	
	int nX = GDALGetRasterBandXSize(hB1);
	int nY = GDALGetRasterBandYSize(hB1);

	/**CUDA STUFF***************/
	int N=nX; 	
	/* Allocate Unified Memory arrays (CUDA 6+)*/
	float *red, *nir, *b7, *water;
	cudaMallocManaged(&red, N*sizeof(float));
	cudaMallocManaged(&nir, N*sizeof(float));
	cudaMallocManaged(&b7, N*sizeof(float));
	cudaMallocManaged(&water, N*sizeof(float));
	
	for(int i=0; i<N;i++)
		water[i]=0.0;

	/* Compute the Blocks of data to be sent to GPU */
	// On GeForce 8600 Galaxy x=256
	// On GeForce 9500 Galaxy & GeForce 9800 GT x=512
	int x=512;
	dim3 dimBlock(x);
	dim3 dimGrid ( (N / dimBlock.x) + (!(N % dimBlock.x)?0:1)) ;
	
	//Accessing the data rowxrow
	for(int row=0;row<nY;row++){
		/* Read input files through GDAL */		
		err = GDALRasterIO(hB1,GF_Read,0,row,nX,1,red,nX,1,GDT_Float32,0,0);
		err = GDALRasterIO(hB2,GF_Read,0,row,nX,1,nir,nX,1,GDT_Float32,0,0);
		err = GDALRasterIO(hB3,GF_Read,0,row/2,nX/2,1,b7,nX,1,GDT_Float32,0,0);
		/* Add arrays red, nir, b7 and store result in water */
		waterid<<<dimGrid,dimBlock>>>(red, nir, b7, water, N);
		/* Block until device completed processing */
		cudaThreadSynchronize();
		/* Write output file through GDAL */
		err = GDALRasterIO(hBOut,GF_Write,0,row,nX,1,water,nX,1,GDT_Float32,0,0);
	}
	/* Free the memory */
	cudaFree(red); 
	cudaFree(nir);
	cudaFree(b7);
	cudaFree(water);
	GDALClose(hD1);
	GDALClose(hD2);
	GDALClose(hD3);
	GDALClose(hDOut);
}
 
