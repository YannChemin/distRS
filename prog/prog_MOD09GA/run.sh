#!/bin/bash
MODproduct=mod09ga
DataRoot=/data/GR/$MODproduct
DataOut=/data/GR/$MODproduct/B3QC/
mkdir -p $DataOut
pre=MOD09GA_
program=$MODproduct
progRoot=$PWD

#Define number of (virtual) cores
ncores=`grep -c 'core id' /proc/cpuinfo`
echo "ncores=" $ncores

#Go to Data directory
cd $DataRoot

for (( doy = 2001000 ; doy <= 2020000 ; doy++ ))
do
	if [ $(( $doy % 1000 )) -le 366 ]
	then
		echo $doy
		# untar files and test for presence of tif files
		test1=$(find . -type f | grep $pre | grep A$doy | grep QC_500m_1 | grep tif | wc -l)
		test2=$(find . -type f | grep $pre | grep A$doy | grep sur_refl_b03 | grep tif |  wc -l)
		if [ $test1 -eq 1 -a $test2 -eq 1 ]
		then
			# get the tif files input/output names
			inB=$(find . -type f | grep A$doy | grep QC_500m_1 | grep tif)
			inB3=$(find . -type f | grep A$doy | grep sur_refl_b03 | grep tif)
			out=$DataOut$pre$doy\_500m_1D_B3_QC_Corrected.tif
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
			echo "./$program $inB $inB3 $out"
			rm -f $out
			$progRoot/$program $inB $inB3 $out &
		fi
	fi
done
