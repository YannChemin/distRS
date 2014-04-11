#include<stdio.h>

void fillin(double *t_sim,double *t_obs, int length, int nodata){
// 	printf("in Fillin part1\n");
	//t,t2 are in temporal dimension
	int t, t2;
	int flag=0;

	//Remove lonely peaks (0.35 NDVI change in 15 days and back)
// 	for (t=1;t<length-1;t++){
// 		if((t_obs[t]-t_obs[t+1])>3500&&(t_obs[t-1]-t_obs[t])<-3500){
// 			t_obs[t]=0.5*(t_obs[t-1]+t_obs[t+1]);
// 		}
// 	}

	//Specific boundary cases FIRST
	if(t_obs[0]==nodata){
		for (t=1;t<length;t++){
			if(flag==0&&t_obs[t]>nodata){
				t_sim[0]=t_obs[t];
				flag=1;
			}
		}
	}
	flag=0;
	//Specific boundary cases LAST
	if(t_obs[length-1]==nodata){
		for (t=length-2;t>0;t--){
			if(flag==0&&t_obs[t]>nodata){
				t_sim[length-1]=t_obs[t];
				flag=1;
			}
		}
	}
	flag=0;
	double previous,next;
// 	Run around the roundabout
	for (t=1;t<length-1;t++){
		//If t_obs[t] is not within bounds then replace it
		//with linear interpolation of closest valid data points
		if(t_obs[t]<=nodata){
			//Find the last previous valid data point
			for (t2=t-1;t2>0;t2--){
				if(flag==0&&t_obs[t2]>nodata){
					previous = t_obs[t2];
					flag=1;
				}
			}
			flag=0;
			//Find the first next valid data point
			for (t2=t+1;t2<length;t2++){
				if(flag==0&&t_obs[t2]>nodata){
					next = t_obs[t2];
					flag=1;
				}
			}
			flag=0;
			t_sim[t]=0.5*(previous+next);
		}
	}
// 	printf("in Fillin part3[%i]\t",u);
}
