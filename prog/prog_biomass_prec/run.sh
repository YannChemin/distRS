#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

preN=mod13q1_
preE=mod16a2_
post=modq1_BP_
ndvi=VI_
et=ETQC_
pet=PETQC_
DataRoot=$(pwd)
in_ndvi=$root/NDVI/
in_et=$root/ET/
in_pet=$root/PET/
out_bp=$root/BP/

mkdir -p $out_bp

#for (( doy = 2001000 ; doy <= 2018000 ; doy++ ))
for (( doy = 2001000 ; doy <= 2016000 ; doy++ ))
do
  test1=$(find $in_ndvi -type f | grep $preN$ndvi | grep $doy | wc -l)
  if [ $test1 -eq 1 ]
  then
	A=$in_ndvi$preN$ndvi$doy.tif
	#A=mod13q1_NDVI_2001049.tif
  fi
  test3=$(find $in_et -type f | grep $preE$et | grep $doy | wc -l)
  test4=$(find $in_pet -type f | grep $preE$pet | grep $doy | wc -l)
  test5=$(find $out_ndvi -type f | grep $out_bp$post$doy.tif | wc -l)
  if [ $test3 -eq 1 -a $test4 -eq 1 -a $test5 -eq 0 ]
  then 	
   	B=$in_et$preE$et$doy.tif
   	C=$in_pet$preE$pet$doy.tif
   	D=$out_bp$post$doy.tif
	#B=mod16a2_ET_2001049.tif
	#C=mod16a2_PET_2001049.tif
	#D=modq1_BP_2001049.tif
	E=$(echo $A | sed 's/mod13q1_VI_....\(.*\).tif/\1/')
	F=0.75
	./bp $A $B $C $D $E $F 
  fi
done

