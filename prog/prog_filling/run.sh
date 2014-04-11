#!/bin/bash

inDIR=~/workspace/RainfallReconst/NDVI_test
outDIR=~/workspace/RainfallReconst/NDVI
mkdir -p $outDIR
rm $outDIR/*.tif

inLISTDIR=$(ls $inDIR/*.tif)

for file in $inLISTDIR
do
	infile=$(echo $file | sed 's/\(.*\)ndvi_\(.*\)/ndvi_\2/')
	outLISTDIR=$outLISTDIR" "$outDIR/$infile 
done

# echo "./filling" $inLISTDIR $outLISTDIR

./filling $inLISTDIR $outLISTDIR
