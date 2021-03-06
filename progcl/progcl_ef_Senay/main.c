#include <stdio.h>
//#include <CL/opencl.h>
#include <gdal.h>
#include <arrays.h>

#define MAX_SOURCE_SIZE (0x100000)
#define MVARS 5

int main(int argc, char* argv[])
{
	char *out	= argv[1];
	char *inlst	= argv[2];
	char *inkernel	= argv[3];
	///GDAL STUFF
	//--------------------------------------------------------
	GDALAllRegister();
	GDALDatasetH hD1	= GDALOpen(inlst,GA_ReadOnly);
	if(hD1==NULL)exit(EXIT_FAILURE);
	GDALDriverH hDr		= GDALGetDatasetDriver(hD1);
	char **options = NULL;
	options = CSLSetNameValue( options, "TILED", "YES" );
	options = CSLSetNameValue( options, "COMPRESS", "DEFLATE" );
	options = CSLSetNameValue( options, "PREDICTOR", "2" );
	GDALDatasetH hDOut 	= GDALCreateCopy(hDr,out,hD1,FALSE,options,NULL,NULL);
	GDALRasterBandH hBOut 	= GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 	= GDALGetRasterBand(hD1,1);
	int nX			= GDALGetRasterBandXSize(hB1);
	int nY			= GDALGetRasterBandYSize(hB1);
	int i, N		= nX * nY;
	float *lst 		= af1d(N);
	float *lOut 		= af1d(N);
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,lst,nX,nY,GDT_Float32,0,0);
	GDALClose(hD1);
	//--------------------------------------------------------
	//SERIAL SECTION
	float t, lst_min=500, lst_max=0;
	for (i=0;i<N;i++){
		t=lst[i]*0.02;
		if(t<lst_min && t>273.15) lst_min=t;
		if(t>lst_max) lst_max=t;
	}
	printf("lst_min=%f\t",lst_min);
	printf("lst_max=%f\n",lst_max);
// 	for (i=0;i<N;i++){
// 		t=lst[i]*0.02;
// 		if(t<lst_min){
// 			lst[i]=lst_min/0.02;
// 		}
// 	}
	for (i=0;i<N;i++){
		t=lst[i]*0.02;
		if(t<lst_min){
			t=lst_min;
		}
		lOut[i]=100*(lst_max-t)/(lst_max-lst_min);
		if(lOut[i]>100.0){
		  lOut[i]=100.0;
		}
	}
	//--------------------------------------------------------
	// OPENCL SECTION
	// Load the kernel source code into the array source_str
// 	FILE *fp 		= fopen(inkernel,"r");
// 	if (!fp){
// 		printf("Failed to load kernel file\n");
// 		exit(EXIT_FAILURE);
// 	}
// 	char *readSource	= (char*)malloc(MAX_SOURCE_SIZE);
// 	fread(readSource,1,MAX_SOURCE_SIZE,fp);
// 	fclose(fp);
// 	const char *sProgSource = (const char *) readSource;
// 	///OpenCL Block structure setup
// 	const unsigned int cnBlockSize = 256;
// 	const unsigned int cnBlocks = (int) ceil(N / cnBlockSize)+1;
// 	const size_t gid0 = cnBlocks * cnBlockSize;
// 	//Create OpenCL Platform ID and Platform Number
// 	cl_platform_id p_id	= NULL;
// 	cl_uint p_num;
// 	cl_int err		= clGetPlatformIDs(1,&p_id,&p_num);
// 	//Get Device ID and Info
// 	cl_device_id d_id	= NULL;
// 	cl_uint d_num;
// 	err			= clGetDeviceIDs(p_id,CL_DEVICE_TYPE_GPU,1,&d_id,&d_num);
// 	cl_context hContext	= clCreateContext(NULL,1,&d_id,NULL,NULL,&err);
// 	//Create OpenCL device and context
// 	//if (err == CL_SUCCESS) printf("Succeeded to create context\n");
// 	if(CL_DEVICE_NOT_FOUND==err){
// 		printf("trying CPU instead of GPU\n");
// 		err		= clGetDeviceIDs(p_id,CL_DEVICE_TYPE_CPU,1,&d_id,&d_num);
// 		hContext	= clCreateContext(NULL,1,&d_id,NULL,NULL,&err);
//  	}
//   	if (err){
//   		printf("Could not find Device to create context on\n");
//   		exit(EXIT_FAILURE);
//   	}
// 	//Query all devices available to the context
// 	size_t nContextDescriptorSize;
// 	clGetContextInfo(hContext, CL_CONTEXT_DEVICES, 0, 0, &nContextDescriptorSize);
// 	cl_device_id * aDevices = malloc(nContextDescriptorSize);
// 	clGetContextInfo(hContext, CL_CONTEXT_DEVICES, nContextDescriptorSize, aDevices, 0);
// 	//Create a command queue for first device the context reported
// 	cl_command_queue hCmdQueue = clCreateCommandQueue(hContext, aDevices[0], 0, 0);
// 	//Create and compile program
// 	cl_program hProg = clCreateProgramWithSource(hContext, 1, &sProgSource, 0, 0);
// 	clBuildProgram(hProg, 0, 0, 0, 0, 0);
// 	//Create Kernel
// 	cl_kernel hKernel = clCreateKernel(hProg, "ef_senay", 0);
// 	//Allocate device memory
// 	cl_mem hDevMem[MVARS-1];
// 	hDevMem[0] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,gid0*sizeof(float),lst,&err);
// 	hDevMem[1] = clCreateBuffer(hContext,CL_MEM_WRITE_ONLY,gid0*sizeof(float),NULL,&err);
//  	hDevMem[2] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(float),&lst_min,&err);
//  	hDevMem[3] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(float),&lst_max,&err);
//  	hDevMem[4] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(int),&N,&err);
//  	//Setup kernel parameter arguments values
// 	for (i=0;i<MVARS;i++){
// 		err = clSetKernelArg(hKernel, i, sizeof(cl_mem), &hDevMem[i]);
// 	}
// 	//Execute Kernel
// 	err = clEnqueueNDRangeKernel(hCmdQueue, hKernel, 1, 0, &gid0, 0, 0, 0, 0);
// 	// Wait for finish commands
// 	clFinish(hCmdQueue);
// 	 //Copy results from device back to host
// // 	err = clEnqueueReadBuffer(hCmdQueue, hDevMem[1], CL_TRUE, 0, N*sizeof(float),lOut,0,NULL,NULL);
// 	//--------------------------------------------------------
// 	for (i=0;i<MVARS;i++){
// 		clReleaseMemObject(hDevMem[i]);
// 	}
// 	free(aDevices);
// 	clReleaseKernel(hKernel);
// 	clReleaseProgram(hProg);
// 	clReleaseCommandQueue(hCmdQueue);
// 	clReleaseContext(hContext);
	//--------------------------------------------------------
	//Copy ndvi to GDAL array and Write ndvi_gdal_array to file
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Float32,0,0);
	free(lst);
	free(lOut);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}
