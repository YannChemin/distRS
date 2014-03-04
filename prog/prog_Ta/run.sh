#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

in_fec=$root/emissivity/
out_fec=$root/FEC/
mkdir $out_fec -p
rm $out_fec/*.tif -f

pre=MOD11A1_

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do 
	test1=$(find $in_fec -type f | grep $pre$doy | wc -l)
	test2=$(find $out_fec -type f | grep $out_fec$pre$doy\_1Km_1D_fec.tif | wc -l)
 	if [ $test1 -eq 2 -a $test2 -eq 0 ]
 	then 
		inB2=$in_fec$pre$doy\_1Km_1D_e31.tif
  		inB3=$in_fec$pre$doy\_1Km_1D_e32.tif
  		outFC=$out_fec$pre$doy\_1Km_1D_fec.tif
		#   echo "./fc $inB2 $inB3 $outFC"
  		echo "./fc@" $doy
  		./fc $inB2 $inB3 $outFC
  	fi
done
