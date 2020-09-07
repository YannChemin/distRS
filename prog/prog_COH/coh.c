#include<stdio.h>
#include<math.h>

//#data1 and data2 are double pointer arrays (w*h) linearized for parallelization
//#Windowed kernel (2x2,3x3,4x4) not more
float coherenceWindowed(int len, float * data1, float * data2){
	double sum1 = 0;
	double sum2 = 0;
	double sum3 = 0;
	for(int i=0; i < len; i++){
		sum1 += data1[i] * data2[i];
		sum2 += data1[i] * data1[i];
		sum3 += data2[i] * data2[i];
	}
	return (fabs(sum1) / 1.0*(sqrt(sum2 * sum3)));
}
//out=coherenceWindowed(data1,data2)
//print(out)
