#!/bin/bash

#------------------------------------------------
#./et_pot_d $inRnetd $inLST
#		$outETPOTD [$roh_w]
#-----------------------------------------------
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/etpotd/
mkdir $outDIR
rm $outDIR/*.tif -f

preRNETD=rnetd_
preLST=MOD11A1_

roh_w=1003.0
for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
 do test1=$(find $root/rnetd/ -type f | grep $preRNETD$doy | wc -l)
  test2=$(find $root/LST/ -type f | grep $preLST$doy | wc -l)
  test3=$(find $outDIR -type f | grep etpotd_$doy.tif | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
   then inRNETD=$root/rnetd/$preRNETD$doy.tif
   echo "found" $doy $inRNETD $inLST
   inLST=$root/LST/$preLST$doy\_1Km_1D_LST.tif
   outETPOTD=$outDIR/etpotd_$doy.tif
#    echo "./et_pot_d $inRNETD $inLST $outETPOTD $roh_w"
   ./et_pot_d $inRNETD $inLST $outETPOTD $roh_w
 fi
done
