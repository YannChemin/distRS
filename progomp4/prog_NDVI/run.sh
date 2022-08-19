#!/bin/bash
# GPU distribution
#Requires: sudo apt install gcc-offload-[nvptx;amdgcn]
#      gcc-offload-amdgcn - GCC offloading compiler to AMD GCN
#      gcc-offload-nvptx - GCC offloading compiler to NVPTX
# in Makefile: -foffload=nvptx-none
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_ndvi=$DataRoot/2_PreProcessed/NDVI/MDB/
in_ndvi_qa=$DataRoot/2_PreProcessed/NDVI_QA/MDB/
out_ndvi=$root/NDVI/
mkdir $out_ndvi
rm $out_ndvi/*.tif -f

pre=MOD13A2_

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $in_ndvi -type f | grep $pre$doy | wc -l)
  test2=$(find $in_ndvi_qa -type f | grep $pre$doy | wc -l)
  test3=$(find $out_ndvi -type f | grep $out_ndvi$pre$doy\_1Km_16D_NDVI.tif | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
  then inB2=$in_ndvi$pre$doy\_1Km_16D_NDVI.tif
   inB3=$in_ndvi_qa$pre$doy\_1Km_16D_NDVI_QA.tif
   outNDVI=$out_ndvi$pre$doy\_1Km_16D_NDVI.tif
   echo "./ndvi $inB2 $inB3 $outNDVI"
   ./ndvi $inB2 $inB3 $outNDVI
  fi
done
