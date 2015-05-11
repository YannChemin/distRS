
/* Test for OpenMP on GPU device
 * Requires GCC 4.9.1 +
 * i.e. OpenMP v4.0 +
 * */

// Makefile
// ---------
// omp4: omp4.c
// 	gcc -o omp4 omp4.c -lm -I/usr/include/gdal -L/usr/lib -lgdal
// run.sh
// ---------
#include<stdio.h>
#include<omp.h>
#include<math.h>
#include"gdal.h"

void usage()
{
	printf( "-----------------------------------------------------------\n");
	printf( "--Calculate the weighted sum of WSA/BSA Albedo images------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "--DFS: Diffuse Fraction of Sunlight------------------------\n");
	printf( "-----------------------------------------------------------\n");
	printf( "./omp4 outALB DFS inWSAfile inBSAfile inQA\n");
	printf( "-----------------------------------------------------------\n");
	return;
}


int main( int argc, char *argv[] )
{
    if( argc > 5 ){
		usage();
		return 1;
	}
	
	#pragma omp declare target
	float F(float);
	#pragma omp end declare target
	
	int ndev = omp_get_num_devices();
	printf("Number of devices: %d\n", ndev);
	int default_device = omp_get_default_device();
	printf("Default device = %d\n", default_device);
	omp_set_default_device(default_device+1);
	if (omp_get_default_device() != default_device+1)
	printf("Default device is still = %d\n", default_device);
	int i;
	int nthreads = omp_is_initial_device() ? 8 : 1024;
	if (!omp_is_initial_device()){
		printf("1024 threads on target device\n");
		nthreads = 1024;
	} else {
		printf("8 threads on initial device\n");
		nthreads = 8;
	}
	return(EXIT_SUCCESS);
}

/*
float x, y;
#pragma omp threadprivate(x, y)
void init(float a, float b){
	#pragma omp single copyprivate(a,b,x,y){
		scanf("%f %f %f %f", &a, &b, &x, &y);
	}
}
*/

/*extern void init(float*, float*, int);
extern void output(float*, int);

void vec_mult(int N){
	int i;
	float p[N], v1[N], v2[N];
	init(v1, v2, N);
	#pragma omp target map(to: v1, v2) map(from: p)
	#pragma omp parallel for
	for (i=0; i<N; i++)
		p[i] = v1[i] * v2[i];
	output(p, N);
}
*/

/*
#pragma omp declare target
float F(float);
#pragma omp end declare target
#define N 1000000000
#define CHUNKSZ 1000000
void init(float *, int);
float Z[N];
void pipedF()
{
	int C, i;
	init(Z, N);
	for (C=0; C<N; C+=CHUNKSZ){
		#pragma omp task
		#pragma omp target map(Z[C:CHUNKSZ])
		#pragma omp parallel for
		for (i=0; i<CHUNKSZ; i++)
			Z[i] = F(Z[i]);
	}
	#pragma omp taskwait
}*/

/*
extern void init(float *, float *, int);
extern void output(float *, int);
void vec_mult(float *p, float *v1, float *v2, int N){
	int i;
	init(v1, v2, N);
	#pragma omp target teams map(to: v1[0:N], v2[:N]) map(from: p[0:N])
	#pragma omp distribute parallel for simd
	for (i=0; i<N; i++)
		p[i] = v1[i] * v2[i];
	output(p, N);
}*/
