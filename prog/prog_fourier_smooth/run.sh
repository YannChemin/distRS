#!/bin/bash

inDIR=cia_NDVI_2000_2010
#donot change this output dir name !
outDIR=out
mkdir -p $outDIR
rm $outDIR/*.tif

inLISTDIR=$(ls $inDIR/*.tif) 

for infile in $inLISTDIR
do
	outLISTDIR=$outLISTDIR" "$(echo $infile | sed 's/\(.*\)\/\(.*\)/out\/\2/') 
done

# echo "./fourier" $inLISTDIR $outLISTDIR

./fourier $inLISTDIR $outLISTDIR