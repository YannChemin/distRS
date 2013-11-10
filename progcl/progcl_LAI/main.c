#include <stdio.h>
#include <CL/opencl.h>
#include <gdal.h>
#include <arrays.h>

#define MAX_SOURCE_SIZE (0x100000)


int main(int argc, char* argv[])
{
	char *inkernel	= argv[1];
	char *out	= argv[2];
	char *inprod	= argv[3];
	char *inprodqa	= argv[4];
	///GDAL STUFF
	//--------------------------------------------------------
	GDALAllRegister();
	GDALDatasetH hD1	= GDALOpen(inprod,GA_ReadOnly);
	GDALDatasetH hD2	= GDALOpen(inprodqa,GA_ReadOnly);
	if(hD1==NULL||hD2==NULL)exit(EXIT_FAILURE);
	GDALDriverH hDr		= GDALGetDatasetDriver(hD1);
	GDALDatasetH hDOut 	= GDALCreateCopy(hDr,out,hD1,FALSE,NULL,NULL,NULL);
	GDALRasterBandH hBOut 	= GDALGetRasterBand(hDOut,1);
	GDALRasterBandH hB1 	= GDALGetRasterBand(hD1,1);
	GDALRasterBandH hB2 	= GDALGetRasterBand(hD2,1);
	int nX			= GDALGetRasterBandXSize(hB1);
	int nY			= GDALGetRasterBandYSize(hB1);
	int N 			= nX * nY;
	unsigned short *prod 	= aus1d(N);
	unsigned short *prodqa 	= aus1d(N);
	unsigned short *lOut 	= aus1d(N);
	GDALRasterIO(hB1,GF_Read,0,0,nX,nY,prod,nX,nY,GDT_Byte,0,0);
	GDALRasterIO(hB2,GF_Read,0,0,nX,nY,prodqa,nX,nY,GDT_Byte,0,0);
	GDALClose(hD1);
	GDALClose(hD2);
	//--------------------------------------------------------
	// OPENCL SECTION
	// Load the kernel source code into the array source_str
	FILE *fp 		= fopen(inkernel,"r");
	if (!fp){
		printf("Failed to load kernel file\n");
		exit(EXIT_FAILURE);
	}
	char *readSource	= (char*)malloc(MAX_SOURCE_SIZE);
	fread(readSource,1,MAX_SOURCE_SIZE,fp);
	fclose(fp);
	const char *sProgSource = (const char *) readSource;
	free(readSource);
	///OpenCL Block structure setup
	const unsigned int cnBlockSize = 256;
	const unsigned int cnBlocks = (int) ceil(N / cnBlockSize)+1;
	const size_t gid0 = cnBlocks * cnBlockSize;
	///Make Mem buffers the rounded size needed for blocks
	unsigned short *bprod 		= aus1d(gid0);
	unsigned short *bprodqa 	= aus1d(gid0);
	int i;
	for (i=0;i<gid0;i++){
		if(i<N){
			bprod[i] = prod[i];
			bprodqa[i] = prodqa[i];
		} else {
			bprod[i] = 0;
			bprodqa[i] = 0;
		}
	}
	//Create OpenCL Platform ID and Platform Number
	cl_platform_id p_id	= NULL;
	cl_uint p_num;
	cl_int err		= clGetPlatformIDs(1,&p_id,&p_num);
	//Get Device ID and Info
	cl_device_id d_id	= NULL;
	cl_uint d_num;
	err			= clGetDeviceIDs(p_id,CL_DEVICE_TYPE_GPU,1,&d_id,&d_num);
	cl_context hContext	= clCreateContext(NULL,1,&d_id,NULL,NULL,&err);
	//Create OpenCL device and context
	if(CL_DEVICE_NOT_FOUND==err){
		printf("trying CPU instead of GPU\n");
		err		= clGetDeviceIDs(p_id,CL_DEVICE_TYPE_CPU,1,&d_id,&d_num);
		hContext	= clCreateContext(NULL,1,&d_id,NULL,NULL,&err);
 	}
  	if (err){
  		printf("Could not find Device to create context on\n");
  		exit(EXIT_FAILURE);
  	}
	//Query all devices available to the context
	size_t nContextDescriptorSize;
	clGetContextInfo(hContext, CL_CONTEXT_DEVICES, 0, 0, &nContextDescriptorSize);
	cl_device_id * aDevices = malloc(nContextDescriptorSize);
	clGetContextInfo(hContext, CL_CONTEXT_DEVICES, nContextDescriptorSize, aDevices, 0);
	//Create a command queue for first device the context reported
	cl_command_queue hCmdQueue = clCreateCommandQueue(hContext, aDevices[0], 0, 0);
	//Create and compile program
	cl_program hProg = clCreateProgramWithSource(hContext, 1, &sProgSource, 0, 0);
	clBuildProgram(hProg, 0, 0, 0, 0, 0);
	//Create Kernel
	cl_kernel hKernel = clCreateKernel(hProg, "clean_mcd15a3_qa", 0);
	//Allocate device memory
	cl_mem hDevMem[3];
 	printf("err = %i\tCL_INVALID_CONTEXT=%i\n",err,CL_INVALID_CONTEXT);
 	printf("err = %i\tCL_INVALID_VALUE=%i\n",err,CL_INVALID_VALUE);
 	printf("err = %i\tCL_INVALID_BUFFER_SIZE=%i\n",err,CL_INVALID_BUFFER_SIZE);
 	printf("err = %i\tCL_INVALID_HOST_PTR=%i\n",err,CL_INVALID_HOST_PTR);
 	printf("err = %i\tCL_MEM_OBJECT_ALLOCATION_FAILURE=%i\n",err,CL_MEM_OBJECT_ALLOCATION_FAILURE);
 	printf("err = %i\tCL_OUT_OF_HOST_MEMORY=%i\n",err,CL_OUT_OF_HOST_MEMORY);
 	printf("err = %i\tCL_INVALID_KERNEL=%i\n",err,CL_INVALID_KERNEL);
	printf("err = %i\tCL_INVALID_ARG_INDEX=%i\n",err,CL_INVALID_ARG_INDEX);
	printf("err = %i\tCL_INVALID_ARG_VALUE=%i\n",err,CL_INVALID_ARG_VALUE);
	printf("err = %i\tCL_INVALID_MEM_OBJECT=%i\n",err,CL_INVALID_MEM_OBJECT);
	printf("err = %i\tCL_INVALID_SAMPLER=%i\n",err,CL_INVALID_SAMPLER);
	printf("err = %i\tCL_INVALID_ARG_SIZE=%i\n",err,CL_INVALID_ARG_SIZE);
	hDevMem[0] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,gid0*sizeof(unsigned short),bprod,&err);
 	printf("Mem0 create clBuf -> err hDevMem[0]=%i\n",err);
	hDevMem[1] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,gid0*sizeof(unsigned short),bprodqa,&err);
 	printf("Mem1 create clBuf -> err hDevMem[1]=%i\n",err);
	hDevMem[2] = clCreateBuffer(hContext,CL_MEM_WRITE_ONLY,gid0*sizeof(unsigned short),NULL,&err);
 	printf("Mem2 create clBuf -> err hDevMem[2]=%i\n",err);
 	hDevMem[3] = clCreateBuffer(hContext,CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR,sizeof(int),&N,&err);
 	printf("Mem3 create clBuf -> err hDevMem[3]=%i\n",err);
 	//Setup kernel parameter arguments values
	err = clSetKernelArg(hKernel, 0, sizeof(cl_mem), &hDevMem[0]);
 	printf("Karg set -> err hDevMem[0]=%i\n",err);
	err = clSetKernelArg(hKernel, 1, sizeof(cl_mem), &hDevMem[1]);
 	printf("Karg set -> err hDevMem[1]=%i\n",err);
	err = clSetKernelArg(hKernel, 2, sizeof(cl_mem), &hDevMem[2]);
 	printf("Karg set -> err hDevMem[2]=%i\n",err);
	err = clSetKernelArg(hKernel, 3, sizeof(cl_mem), &hDevMem[3]);
 	printf("Karg set -> err hDevMem[3]=%i\n",err);
	//Execute Kernel
	err = clEnqueueNDRangeKernel(hCmdQueue, hKernel, 1, 0, &gid0, 0, 0, 0, 0);
	printf("Enqueue NDRange -> err =%i\n",err);
	// Wait for finish commands
	clFinish(hCmdQueue);
	//Copy results from device back to host
	err = clEnqueueReadBuffer(hCmdQueue, hDevMem[2], CL_TRUE, 0, gid0*sizeof(int),lOut,0,NULL,NULL);
 	printf("Enqueue ReadBuf -> err =%i\n",err);
	//--------------------------------------------------------
	clReleaseMemObject(hDevMem[0]);
	clReleaseMemObject(hDevMem[1]);
	clReleaseMemObject(hDevMem[2]);
	clReleaseMemObject(hDevMem[3]);
	free(aDevices);
	clReleaseKernel(hKernel);
	clReleaseProgram(hProg);
	clReleaseCommandQueue(hCmdQueue);
	clReleaseContext(hContext);
	//--------------------------------------------------------
	//Copy prod to GDAL array and Write prod_gdal_array to file
	GDALRasterIO(hBOut,GF_Write,0,0,nX,nY,lOut,nX,nY,GDT_Byte,0,0);
	free(prod);
	free(prodqa);
	free(bprod);
	free(bprodqa);
	free(lOut);
	GDALClose(hDOut);
	return(EXIT_SUCCESS);
}
