__kernel void clean_mcd15a3_qa( __global const unsigned short * lai, __global const unsigned short * laiqa, __global const unsigned short * lai_clean, __global const int * N){
	int i = get_global_id(0);
	if(i<N){
		laiqa[i] >> 15;
		lai_clean[i] = (1-laiqa[i])?255:lai[i];
	}
}