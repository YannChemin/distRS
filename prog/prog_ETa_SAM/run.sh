#!/bin/bash

#------------------------------------------------
#./sam_eta $inLst $inEtpotd
#		$outSAM_ETA $outSAM_EVAPFR
#-----------------------------------------------
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

outDIR0=$root/evapfr_sam/
outDIR=$root/eta_sam/
mkdir $outDIR0
mkdir $outDIR
rm $outDIR0/*.tif -f
rm $outDIR/*.tif -f

preETPOT=etpotd_
preLST=MOD11A1_

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do
	#Test for presence of input files
	test1=$(find $root/etpotd/ -type f | grep $preET0PM$doy | wc -l)
	test2=$(find $root/LST/ -type f | grep $preLST$doy | wc -l)
	if [ $test1 -eq 1 -a $test2 -eq 1 ]
	then
		#Report DOY and input files
		echo "found" $doy $inET0PM $inLST
		#Set input files variables
		inETPOT=$root/etpotd/$preETPOT$doy.tif
		inLST=$root/LST/$preLST$doy\_1Km_1D_LST.tif
		outSAM_EVAPFR=$outDIR0/evapfr_sam_$doy.tif
		outSAM_ETA=$outDIR/eta_sam_$doy.tif
		#Run the algorithm on that date
		./sam_eta $inLST $inETPOT $outSAM_EVAPFR $outSAM_ETA
	fi
done
