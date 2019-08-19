#!/bin/bash
#-----------------------------------------
#--Modis Processing chain--Serial code----
#-----------------------------------------
#./ndvi inB1 inB2 inQC inB3
#        outNDVI
#-----------------------------------------
#inB1/B2         Modis MOD09GQ B1 B2 250m
#inQC            Modis MOD09GQ QC 250m
#inB3            Modis MOD09GA B3 QC corrected 250m
#outNDVI QA corrected NDVI 250m output [-]
#-----------------------------------------

MODGAproduct=mod09ga
MODGQproduct=mod09gq
DataRootGA=/data/GR/$MODGAproduct
DataRootGQ=/data/GR/$MODGQproduct
DataOut=/data/GR/MOD_NDVI/
mkdir -p $DataOut
preGA=MOD09GA_
preGQ=MOD09GQ_
program=ndvi
progRoot=$PWD

#Define number of (virtual) cores
ncores=`grep -c 'core id' /proc/cpuinfo`
echo "ncores=" $ncores

for (( doy = 2001000 ; doy <= 2020000 ; doy++ ))
do
	if [ $(( $doy % 1000 )) -le 366 ]
	then
		echo $doy
		#Go to MOD09GQ Data directory
		cd $DataRootGQ
		# test for presence of tif files
		test1=$(find . -type f | grep $preGQ | grep A$doy | grep QC_250m_1 | grep tif | wc -l)
		test2=$(find . -type f | grep $preGQ | grep A$doy | grep sur_refl_b01 | grep tif |  wc -l)
		test3=$(find . -type f | grep $preGQ | grep A$doy | grep sur_refl_b02 | grep tif |  wc -l)
		if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 1 ]
		then
			# get the tif files input/output names
			inBQC250m=$DataRootGQ/$(find . -type f | grep A$doy | grep QC_500m_1 | grep tif)
			inB1=$DataRootGQ/$(find . -type f | grep A$doy | grep sur_refl_b01 | grep tif)
			inB2=$DataRootGQ/$(find . -type f | grep A$doy | grep sur_refl_b02 | grep tif)
		fi
		#Go to MOD09GA Data directory
		cd $DataRootGA
		# test for presence of tif files
		test1=$(find . -type f | grep $preGQ | grep A$doy | grep QC_250m_1 | grep tif | wc -l)
		if [ $test1 -eq 1 ]
		then
			# get the tif files input/output names
			inBQC500m=$DataRootGQ/$(find . -type f | grep A$doy | grep QC_500m_1 | grep tif)
		fi
		out=$DataOut$preGQ$doy\_250m_1D_NDVI_QC_250_500_Corrected.tif
		#define number of prog running and add ncores
		# So 2 prog will run simultaneously on each core
		npid=$(echo "$(ps aux | grep $program | wc -l) * 2 - 1" | bc)
		while [ $npid -ge $ncores ]
		do
			sleep 1
			#update number of prog running
			npid=$(echo "$(ps aux | grep $program | wc -l) * 2 - 1" | bc)
			#update number of (virtual) cores (for heterogeneous systems)
			ncores=`grep -c 'core id' /proc/cpuinfo`
		done
		# process the thing
		echo "./$program $inB1 $inB2 $inBQC250m $inBQC500m $out"
		rm -f $out
		$progRoot/$program $inB1 $inB2 $inBQC250m $inBQC500m $out &
	fi
done
