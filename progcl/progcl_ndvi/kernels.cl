//Define Kernel
__kernel void nd_vi( __global const float * red, __global const float * nir, __global float * ndvi, __global const int * N){
        int i = get_global_id(0);
        if(i<N)
                ndvi[i] = (nir[i]-red[i])/(nir[i]+red[i]);
}
