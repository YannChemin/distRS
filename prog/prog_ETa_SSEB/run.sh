#!/bin/bash

#------------------------------------------------
#./sseb_eta $inLst $inEt0pm
#		$outSSEB_ETA $outSSEB_EVAPFR $outSSEB_THETA
#-----------------------------------------------
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

outDIR1=$root/theta_sseb/
outDIR0=$root/evapfr_sseb/
outDIR=$root/eta_sseb/
mkdir $outDIR1
mkdir $outDIR0
mkdir $outDIR
rm $outDIR1/*.tif -f
rm $outDIR0/*.tif -f
rm $outDIR/*.tif -f

preET0PM=eto_pm_
preLST=MOD11A1_

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
  do test1=$(find $root/eto_pm/ -type f | grep $preET0PM$doy | wc -l)
   test2=$(find $root/LST/ -type f | grep $preLST$doy | wc -l)
   test3=$(find $outDIR0 -type f | grep evapfr_sseb_$doy.tif | wc -l)
   test4=$(find $outDIR -type f | grep eta_sseb_$doy.tif | wc -l)
   test5=$(find $outDIR1 -type f | grep theta_sseb_$doy.tif | wc -l)
   if [ $test1 -eq 1 -a $test2 -eq 1 ]
   then echo "found" $doy $inET0PM $inLST
   inET0PM=$root/eto_pm/$preET0PM$doy.tif
   inLST=$root/LST/$preLST$doy\_1Km_1D_LST.tif
   outSSEB_EVAPFR=$outDIR0/evapfr_sseb_$doy.tif
   outSSEB_ETA=$outDIR/eta_sseb_$doy.tif
   outSSEB_THETA=$outDIR1/theta_sseb_$doy.tif
#    echo "./sseb_eta $inLST $inET0PM $outSSEB_EVAPFR $outSSEB_ETA $outSSEB_THETA"
   ./sseb_eta $inLST $inET0PM $outSSEB_EVAPFR $outSSEB_ETA $outSSEB_THETA
 fi
done
