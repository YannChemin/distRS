#!/bin/bash

yearSTART=2000
yearEND=2016
doySTART=2000000
doyEND=2016000

#Fill temporally the datasets

#Gap filling directories
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR

#Gap filling for eta data
for (( year=$yearSTART; year<=$yearEND; year++ ))
do
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/eta_sebal_$year*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)eta_sebal_\(.*\)/eta_sebal_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/eta_sebal_*.tif ~/DATA/
done

#Gap filling for LST data
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR
for (( year=$yearSTART; year<=$yearEND; year++ ))
do
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/lst_$year*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)lst_\(.*\)/lst_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make clean
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/lst_*.tif ~/DATA/
done

#Gap filling for NDVI data
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR
inLISTDIR=""
outLISTDIR=""
inLISTDIR=$(ls $inDIR/ndvi_*.tif)
for file in $inLISTDIR
do
 infile=$(echo $file | sed 's/\(.*\)ndvi_\(.*\)/ndvi_\2/')
 outLISTDIR=$outLISTDIR" "$outDIR/$infile
done
cd ~/dev/distRS/trunk/prog/prog_filling
make clean
make
./filling $inLISTDIR $outLISTDIR
mv ~/DATA/ta_int/ndvi_*.tif ~/DATA/

#Gap filling for ALB data
 inDIR=~/DATA
 outDIR=~/DATA/ta_int
 mkdir -p $outDIR
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/alb_*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)alb_\(.*\)/alb_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make clean
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/alb_*.tif ~/DATA/


