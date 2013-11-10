#!/bin/bash
# 	-----------------------------------------
# 	--Modis Processing chain--Serial code----
# 	-----------------------------------------
# 	./t0dem inDEM inLST inLST_QA
# 	outT0DEM
# 	-----------------------------------------
# 	inDEM Digital Elevation Model 250m [m]
# 	inLST Modis LST day 1Km
# 	inLST_QA Modis LST day 1Km Quality Assessment
#
# 	outT0DEM Altitude corrected Temperature output [K]

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_lst=$DataRoot/2_PreProcessed/LST/MDB/
in_lst_qa=$DataRoot/2_PreProcessed/LST_QA/MDB/
out_lst=$root/t0dem/
mkdir $out_lst -p
rm $out_lst/*.tif -f

pre=MOD11A1_
inDEM=$DataRoot/1_HDF/DEM/srtm.tif

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $in_lst -type f | grep $pre$doy | wc -l)
  test2=$(find $in_lst_qa -type f | grep $pre$doy | wc -l)
  test3=$(find $out_lst -type f | grep $out_lst$pre$doy\_t0dem.tif | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 ]
  then inB2=$in_lst$pre$doy\_1Km_1D_LST.tif
   inB3=$in_lst_qa$pre$doy\_1Km_1D_LST_QA.tif
   outT0DEM=$out_lst$pre$doy\_t0dem.tif
   echo "./t0dem" $inDEM $inB2 $inB3 $outT0DEM
   ./t0dem $inDEM $inB2 $inB3 $outT0DEM
  fi
done
