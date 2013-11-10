#include<stdio.h>

void movavg(double *t_sim,double *t_obs, int length){
	//t,t2 are in temporal dimension
	int t;
	//Specific boundary cases FIRST
	t_sim[0]=0.5*(t_obs[0]+t_obs[1]);
	//Specific boundary cases LAST
	t_sim[length-1]=0.5*(t_obs[length-2]+t_obs[length-3]);
// 	Run around the roundabout
	for (t=1;t<length-1;t++){
		//If t_obs[t] is not within bounds then replace it 
		//with moving average of closest valid data points
		t_sim[t]=(t_obs[t-1]+0.5*t_obs[t]+t_obs[t+1])/2.5;
	}
}
