#!/bin/bash

root=$(pwd)
working_root=processed/
mkdir -p $working_root

for file in *LST_LST*.tif
do
	../../prog_opencl_ef_Senay/ef_senay $working_root/ef_$file $file ../../prog_opencl_ef_Senay/kernels.cl
done