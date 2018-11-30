#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_et=$DataRoot/2_PreProcessed/ET/
in_pet=$DataRoot/2_PreProcessed/PET/
in_qa=$DataRoot/2_PreProcessed/QA/
out_evapfr=$root/EVAPFR/
mkdir $out_evapfr
rm $out_evapfr/*.tif -f

pre=MOD16A2_

for (( doy = 2010000 ; doy <= 2019000 ; doy++ ))
do test0=$(find $in_et -type f | grep $pre$doy | wc -l)
  test1=$(find $in_pet -type f | grep $pre$doy | wc -l)
  test2=$(find $in_qa -type f | grep $pre$doy | wc -l)
  test3=$(find $out_evapfr -type f | grep $out_evapfr$pre$doy\_500m_8D_EVAPFR.tif | wc -l)
  if [ $test0 -eq 1 -a $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
  then inB1=$in_et$pre$doy\_500m_8D_ET.tif
   inB2=$in_pet$pre$doy\_500m_8D_PET.tif
   inB3=$in_qa$pre$doy\_500m_8D_QA.tif
   outEVAPFR=$out_evapfr$pre$doy\_500m_8D_EVAPFR.tif
   echo "./evapfr $inB2 $inB3 $outEVAPFR"
   ./evapfr $inB2 $inB3 $outEVAPFR
  fi
done
