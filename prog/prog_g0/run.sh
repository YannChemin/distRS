#!/bin/bash

# -----------------------------------------
# --Modis Processing chain--Serial code----
# -----------------------------------------
# ./g_0 inAlbedo inNdvi inLst inRnet inTime
#         outG0
#         [inRoerink]
# -----------------------------------------
# outG0   Soil Heat Flux output [W/m2]
# inRoerink       Use Roerink Hapex-Sahel calibration [0;1]

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/g0/
mkdir $outDIR
rm $outDIR/*.tif -f
preALB=MCD43B3_
preNDV=MOD13A2_
preLST=MOD11A1_

roerink=0
for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
  do test1=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
  test2=$(find $root/NDVI/ -type f | grep $preNDV$doy | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 ]
   then inAlb=$root/albedo/$preALB$doy\_1Km_16D_ALB.tif
   inNdvi=$root/NDVI/$preNDV$doy\_1Km_16D_NDVI.tif
   echo "found" $doy $inAlb $inNdvi
   for (( ddoy = doy ; ddoy <= doy+15 ; ddoy++ ))
   do test3=$(find $root/rnet/ -type f | grep rnet_$ddoy | wc -l)
    test4=$(find $root/time/ -type f | grep $preLST$ddoy | wc -l)
    test5=$(find $root/LST/ -type f | grep $preLST$ddoy | wc -l)
    test6=$(find $outDIR -type f | grep g0_$ddoy.tif | wc -l)
    if [ $test3 -eq 1 -a $test4 -eq 1 -a $test5 -eq 1 -a $test6 -eq 0 ]
     then echo "Found Daily@" $ddoy
     inRnet=$root/rnet/rnet_$ddoy.tif
     inTime=$root/time/$preLST$ddoy\_1Km_1D_time.tif
     inLst=$root/LST/$preLST$ddoy\_1Km_1D_LST.tif
     outG0=$outDIR/g0_$ddoy.tif
#      echo "./g_0 $inAlb $inNdvi $inLst $inRnet $inTime $outG0 $roerink"
     ./g_0 $inAlb $inNdvi $inLst $inRnet $inTime $outG0 $roerink
    fi
   done
  fi
done
