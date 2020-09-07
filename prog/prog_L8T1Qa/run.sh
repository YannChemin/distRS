#!/bin/bash

if [ $# -lt 5 ]; then
  echo 1>&2 "$0: not enough arguments: number of threads & output directory"
  exit 2
elif [ $# -gt 5 ]; then
  echo 1>&2 "$0: too many arguments"
  exit 2
fi

# Recompile with latest changes
make clean
make

# Number of running threads
export OMP_NUM_THREADS=$1

#Basename of satellite imagery tile without Path and Row
productL8=LC08_L1TP_142054_

#PWD program
PWD=$(pwd)

#RSDATA directory (sub) structure
DataRoot=$PWD/../RSDATA
root=$DataRoot/3_Products
in_l8=$DataRoot/2_PreProcessed/L8/
in_l8_qa=$DataRoot/2_PreProcessed/L8/
out_l8=$root/$2/

#Make output directory
mkdir -p $out_l8
rm $out_l8/*.tif -f

#Process by timestamp range
for (( doy = 20180429 ; doy <= 20180429 ; doy++ ))
do 	test0=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep tar.gz | wc -l)
	if [ $test0 -eq 1 ] 
	then 
		# Uncompress tarball
		tar xvf $(find $in_l8 -type f | grep $productL8 | grep $doy | grep tar.gz) -C $in_l8 
		# Get input files count
		testb4=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B4.TIF | wc -l)
		testb5=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B5.TIF | wc -l)
		testb6=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B6.TIF | wc -l)
		testqa=$(find $in_l8_qa -type f | grep $productL8 | grep $doy | grep _BQA.TIF | wc -l)
		# Get output file count
  		test3=$(find $out_l8 -type f | grep $out_l8$productL8\_$doy\_NDVI.tif | wc -l)
		#if output exists, do not overwrite (test3 -eq 0)
		if [ $testb4 -eq 1 -a $testb5 -eq 1 -a $testb6 -eq 1 -a $testqa -eq 1 -a $test3 -eq 0 ]
  		then 
			# Get Band 4/5/6
			inB4=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B4.TIF)
			inB5=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B5.TIF)
			inB6=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _B6.TIF)
			# Get QA band
			inB3=$(find $in_l8 -type f | grep $productL8 | grep $doy | grep _BQA.TIF)
			# Set output name
			outL8VI=$out_l8$productL8\_$doy\_NDVI.tif
			outL8WI=$out_l8$productL8\_$doy\_NDWI.tif
			# Process
			#echo "$PWD/l8t1qa $inB4 $inB5 $inB6 $inB3 $outL8VI $outL8WI" 
			#time $PWD/l8t1qa $inB4 $inB5 $inB6 $inB3 $outL8VI $outL8WI
			echo "python ./run.py $inB4 $inB5 $inB6 $inB3 $outL8VI $outL8WI" 
			python ./run.py $inB4 $inB5 $inB6 $inB3 $outL8VI $outL8WI 
			# Clean up
			rm -f $in_l8/*.TIF
			rm -f $in_l8/*.txt
			# Tarball the output file & clean up
			cd $out_l8
			echo "cd $out_l8"
			for file in *.tif
			do	tar -cvzf $(echo $file| sed 's/.tif//').tar.gz $file $(echo $file | sed 's/.tif/.IMD/') -C $out_l8
				rm -f $file
				rm -f $(echo $file | sed 's/.tif/.IMD/')
			done
		fi 
	fi
done
