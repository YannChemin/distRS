#!/bin/bash
DataRoot=/RSDATA/lst_terra

pre=MOD11A2_

#Define number of (virtual) cores
ncores=`grep -c 'core id' /proc/cpuinfo`
echo "ncores=" $ncores


for (( doy = 2000000 ; doy <= 2020000 ; doy++ ))
do
	#Test presence of tar.gz files at the given date
	test1=$(find $DataRoot -type f | grep A$doy | grep LST_LST | grep tar.gz | wc -l)
	test2=$(find $DataRoot -type f | grep A$doy | grep LST_QC | grep tar.gz |  wc -l)
	test3=$(find $DataRoot -type f | grep $pre$doy\_1Km_1D_LST.tar.gz | wc -l)
	echo "level1 $test1 $test2 $test3"
	if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 0 ]
	then
		# untar files and test for presence of tif files
		tar xvf $(find $DataRoot -type f | grep A$doy | grep LST_LST | grep tar.gz)
		tar xvf $(find $DataRoot -type f | grep A$doy | grep LST_QC | grep tar.gz)
		test1=$(find . -type f | grep A$doy | grep LST_LST | grep tif | wc -l)
		test2=$(find . -type f | grep A$doy | grep LST_QC | grep tif |  wc -l)
		echo "level2 $test1 $test2" 
		if [ $test1 -eq 1 -a $test2 -eq 1 ]
		then
			# get the tif files input/output names
			inB2=$(find . -type f | grep A$doy | grep LST_LST | grep tif)
			inB3=$(find . -type f | grep A$doy | grep LST_QC | grep tif)
			outLST=$pre$doy\_1Km_8D_LST.tif
			echo $inB1
			echo $inB2
			echo $outLST
			#define number of prog running and add ncores
			# So 2 prog will run simultaneously on each core
			npid=$(echo "$(ps aux | grep lst | wc -l) * 2 - 1" | bc)
			while [ $npid -ge $ncores ]
			do
				sleep 1
				#update number of prog running
				npid=$(echo "$(ps aux | grep lst | wc -l) * 2 - 1" | bc)
				#update number of (virtual) cores (for heterogeneous systems)
				ncores=`grep -c 'core id' /proc/cpuinfo`
			done
			# process the thing, compress output and delete tif output file
			echo "./lst $inB2 $inB3 $outLST"
			./lst $inB2 $inB3 $outLST
			#rm -f $inB2 &
			#rm -f $inB3 &
			#tar czf $(echo $DataRoot$outLST | sed 's/\.tif//g').tar.gz $outLST
			#rm -f $outLST &
		fi
	fi
done
