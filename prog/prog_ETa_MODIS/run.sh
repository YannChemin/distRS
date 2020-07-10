#!/bin/bash

productETA=MOD16A2H_

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_eta=$DataRoot/2_PreProcessed/ETA/MDB/
in_eta_qa=$DataRoot/2_PreProcessed/ETA_QA/MDB/
out_eta=$root/ETA/

mkdir $out_eta
rm $out_eta/*.tif -f


for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
 do test1=$(find $in_eta -type f | grep $productETA$doy | wc -l)
  test2=$(find $in_eta_qa -type f | grep $productETA$doy | wc -l)
  test3=$(find $out_eta -type f | grep $out_eta$productETA$doy\_500m_16D_ETA.tif | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
  then inB2=$in_eta$productETA$doy\_500m_16D_ETA.tif 
   inB3=$in_eta_qa$productETA$doy\_500m_16D_ETA_QA.tif 
   outETA=$out_eta$productETA$doy\_500m_16D_ETA.tif 
   echo "./eta $inB2 $inB3 $outETA" 
  ./eta $inB2 $inB3 $outETA 
  fi 
done
