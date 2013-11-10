#!/bin/bash

productLAI=MCD15A2_

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_lai=$DataRoot/2_PreProcessed/LAI/MDB/
in_lai_qa=$DataRoot/2_PreProcessed/LAI_QA/MDB/
out_lai=$root/LAI/

mkdir $out_lai
rm $out_lai/*.tif -f


for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
 do test1=$(find $in_lai -type f | grep $productLAI$doy | wc -l)
  test2=$(find $in_lai_qa -type f | grep $productLAI$doy | wc -l)
  test3=$(find $out_lai -type f | grep $out_lai$productLAI$doy\_1Km_8D_LAI.tif | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
  then inB2=$in_lai$productLAI$doy\_1Km_8D_LAI.tif 
   inB3=$in_lai_qa$productLAI$doy\_1Km_8D_LAI_QA.tif 
   outLAI=$out_lai$productLAI$doy\_1Km_8D_LAI.tif 
   echo "./lai $inB2 $inB3 $outLAI" 
  ./lai $inB2 $inB3 $outLAI 
  fi 
done
