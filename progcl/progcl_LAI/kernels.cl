
//Define Kernels
//This is an initial test kernel, kept here for historical reasons (NDVI=(nir-red)/(nir+red))
__kernel void nd_vi( __global const float * red, __global const float * nir, __global float * ndvi, __global const int * N){
	int i = get_global_id(0);
	if(i<N)
		ndvi[i] = (nir[i]-red[i])/(nir[i]+red[i]);
}

//MODIS Quality flags for cleaning NDVI products (MOD13A2)
__kernel void clean_mod13a2_qa( __global const float * ndvi, __global const float * ndviqa, __global float * ndvi_clean, __global const int * N){
	int i = get_global_id(0);
	if(i<N){
		if( ndviqa[i] > 1){
			ndvi_clean[i] = -28768;
		} else {
			ndvi_clean[i] = ndvi[i];
		}
	}
}

//MODIS Quality flags for cleaning NDVI products (MCD43B2/B3)
__kernel void clean_mcd43b3_qa( __global const float * albedowsa, __global const float * albedobsa, __global const float * albedoqa, __global float * albedo_clean, __global const int * N){
	int i = get_global_id(0);
	if(i<N)
		albedo_clean[i] = (albedowsa[i] * 0.25 + (1-0.25)*albedobsa[i])*0.1;
		if( albedoqa[i] > 1) albedo_clean[i] = -28768;
}


//MODIS Quality flags for cleaning LST products (MOD11A1)
__kernel void clean_mod11a1_qa( __global const int * lst, __global const int * lstqa, __global int * lst_clean, __global const int * N){
	int i = get_global_id(0);
	if(i<N)
		lst_clean[i] = lst[i];
		if( (lstqa[i] & 0x03) > 1 ) lst_clean[i] = -28768;
}

//MODIS Quality flags for cleaning LAI products (MCD15A3)
__kernel void clean_mcd15a3_qa( __global const float * lai, __global const int * laiqa, __global float * lai_clean, __global const int * N){
	int i = get_global_id(0);
	if(i<N){
		if( laiqa[i] & 0x01 ){
			lai_clean[i] = -28768;
		} else {
			lai_clean[i] = lai[i];
		}
	}
}