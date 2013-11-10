#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

in_fc=$root/emissivity/
out_fc=$root/FC/
mkdir $out_fc -p
rm $out_fc/*.tif -f

pre=MOD11A1_

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $in_fc -type f | grep $pre$doy | wc -l)
 test2=$(find $out_fc -type f | grep $out_fc$pre$doy\_1Km_1D_FC.tif | wc -l)
 if [ $test1 -eq 2 -a $test2 -eq 0 ]
 then inB2=$in_fc$pre$doy\_1Km_1D_e31.tif
  inB3=$in_fc$pre$doy\_1Km_1D_e32.tif
  outFC=$out_fc$pre$doy\_1Km_1D_FC.tif
#   echo "./fc $inB2 $inB3 $outFC"
  echo "./fc@" $doy
  ./fc $inB2 $inB3 $outFC
  fi
done
