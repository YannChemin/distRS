#!/bin/bash

#------------------------------------------------
#-----------------------------------------
# --Modis Processing chain--OpenMP code----
# -----------------------------------------
# ./pm_eto inLst inDem inRnet
#         outPm_eto
# -----------------------------------------
#         rh u
#-----------------------------------------------

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/eto_pm/
mkdir $outDIR
rm $outDIR/*.tif -f

preLST=MOD11A1_
inDEM=$DataRoot/1_HDF/DEM/srtm.tif

RH=0.30
u=3.0
for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $root/LST/ -type f | grep $preLST$doy | wc -l)
 test2=$(find $root/rnet/ -type f | grep rnet_$doy | wc -l)
 test3=$(find $outDIR -type f | grep eto_pm_$doy.tif | wc -l)
 if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
  then inLST=$root/LST/$preLST$doy\_1Km_1D_LST.tif
  inRNET=$root/rnet/rnet_$doy.tif
  outPM_ETO=$outDIR/eto_pm_$doy.tif
  echo "found" $doy $inLST $inRNET
#   echo "./pm_eto $inLST $inDEM $inRNET $outPM_ETO $RH $u"
   ./pm_eto $inLST $inDEM $inRNET $outPM_ETO $RH $u
 fi
done

