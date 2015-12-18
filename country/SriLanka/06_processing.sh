#!/bin/bash

doySTART=2000000
doyEND=2016000

#Go to Data Directory
cd ~/DATA

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores

PRoot=~/dev/distRS/trunk

#Vegetation Fraction
prog_root=$PRoot/prog/prog_fc
cd $prog_root
make clean
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		#echo "Vegetation Fraction" $doy
		#count GeoTiff identified
 		c1=$(find -type f | grep ndvi_  | grep $doy | grep .tif | wc -l)
 		if [[ $c1 == 1 ]]
 		then
 			#NDVI file
 			#find GeoTiff
 			f1=$(find -type f | grep ndvi_  | grep $doy | grep .tif)
 			#Output filename
 			out=fc_$doy.tif
 			#does it already exist?
 			outno=$(find -type f | grep $out | wc -l)
 			#Define number of instances running
 			npid=$(echo "$(ps aux | grep fc\  | wc -l) - 1" | bc)
 			while [ $npid -ge $ncores ]
 			do
 				sleep 1
 				#Update number of instances running
 				npid=$(echo "$(ps aux | grep fc\  | wc -l) - 1" | bc)
 				#Update number of (virtual) cores (for heterogeneous systems)
 				ncores=`grep -c 'processor' /proc/cpuinfo`
 			done
 			echo -e "\e[01;34m"
 			echo "fc" $f1 $out 
 			echo -e "\e[00m"
 			#process
 			$prog_root/fc $f1 $out &
 		fi
	fi
done

#Transpiration Fraction
prog_root=$PRoot/prog/prog_Ta
cd $prog_root
make clean
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		#echo "Transpiration Fraction" $doy
		#count GeoTiff identified
 		c1=$(find -type f -name "eta_sebal_*.tif"  | grep $doy | wc -l)
		echo "c1=" $c1
 		if [[ $c1 == 1 ]]
 		then
 			#ETa file
 			#find GeoTiff
 			f1=$(find -type f -name "eta_sebal_*.tif" | grep $doy)
 			#count GeoTiff identified
			c2_old=$c2
			echo "c2_old=" $c2
 			c2=$(find -type f -name "fc_*.tif" | grep $doy | wc -l)
			echo "c2=" $c2
 			if [[ $c2 == 1 ]]
 			then
 				#fc file
 				#find GeoTiff
 				f2=$(find -type f -name "fc_*.tif" | grep $doy)
			fi
 			#Output filename
 			out=ta_$doy.tif
 			#does it already exist?
 			outno=$(find -type f | grep $out | wc -l)
 			#Define number of gdalwarp running
 			npid=$(echo "$(ps aux | grep ta\  | wc -l) - 1" | bc)
			#echo "npid=" $npid
 			while [ $npid -ge $ncores ]
 			do
 				sleep 1
 				#Update number of tfr running
 				npid=$(echo "$(ps aux | grep ta\  | wc -l) - 1" | bc)
 				#Update number of (virtual) cores (for heterogeneous systems)
 				ncores=`grep -c 'processor' /proc/cpuinfo`
 			done
 			echo -e "\e[01;34m"
 			echo "ta" $f2 $f1 $out 
 			echo -e "\e[00m"
 			#process
 			$prog_root/ta $f2 $f1 $out &
 		fi
	fi
done

