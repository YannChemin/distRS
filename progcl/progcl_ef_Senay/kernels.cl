
//Define Kernel
__kernel void ef_senay( __global const float * lst, __global float * ef, __global const float * lst_min, __global const float * lst_max, __global const int * N){
	int i = get_global_id(0);
	if(i<N)
		ef[i] = (lst_max-(0.02*lst[i]))/(lst_max-lst_min);
}
