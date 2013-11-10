#include<stdio.h>
#include<math.h>

#define PI 3.1415926
#define HARMONIC_MAX 366*2


void fourier(double *t_sim,double *t_obs,int length,int harmonic_number){
// 	printf("in Fourier part1\n");
	int u, t;
// 	printf("in Fourier part2\n");
	double fcos[HARMONIC_MAX] = {0.0};
// 	printf("in Fourier part3\n");
	double fsin[HARMONIC_MAX] = {0.0};
// 	printf("in Fourier part4\n");
	double fm[HARMONIC_MAX] = {0.0};
// 	printf("in Fourier part5\n");
	double fp[HARMONIC_MAX] = {0.0};
// 	printf("in Fourier part6\n");
	//Generate F[u], Fm[u] and Fp[u] for u=1 to q
	//u is spectral dimension
	//t is temporal dimension
	for (u=0;u<harmonic_number;u++){
// 	printf("in Fourier part7[%i]\t",u);
		for (t=0;t<length;t++){
			fcos[u] = fcos[u]+t_obs[t]*cos(2*PI*u*t/length);
			fsin[u] = fsin[u]+t_obs[t]*sin(2*PI*u*t/length);
		}
// 		printf("%i\n",u);
		fcos[u]	= fcos[u]/length;
		fsin[u]	= fsin[u]/length;
		fm[u]	= pow(pow(fcos[u],2)+pow(fsin[u],2),0.5);
		fp[u]	= atan2(fcos[u],fsin[u]);
// 		printf("fcos[%i]=%7.3f\tfsin[%i]=%7.3f\tfm[%i]=%7.3f\tfp[%i]=%7.3f\n",u,fcos[u],u,fsin[u],u,fm[u],u,fp[u]);
	}
// 	printf("in Fourier part8\n");
	for (t=0;t<length;t++){
// 		printf("in Fourier part9[%i]\t",t);
		for (u=0;u<harmonic_number;u++){
			t_sim[t] = t_sim[t]+fm[u]*(cos((2*PI*u*t/length)-fp[u])+sin((2*PI*u*t/length)+fp[u]));
		}
	}
// 	printf("in Fourier part10[%i]\t",u);
}
