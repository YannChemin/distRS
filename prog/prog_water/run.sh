#!/bin/bash

doySTART=2000000
doyEND=2015000

#Go to Data Directory
cd ~/DATA

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores

PRoot=~/dev/distRS/trunk

#RNET PROCESSING
prog_root=$PRoot/prog/prog_water
cd $prog_root
make clean
make
cd ~/DATA

for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "Water" $doy
		if [ $(find -type f | grep MOD09Q1_ | grep sur_refl_b01 | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			 #Update B01@250m file
			 f1=$(find -type f | grep MOD09Q1_ | grep sur_refl_b01 | grep A$doy | grep .tif)
			 #count GeoTiff identified
			 c1=$(find -type f | grep MOD09Q1_ | grep sur_refl_b01 | grep A$doy | grep .tif | wc -l)
		fi
		if [ $(find -type f | grep MOD09Q1_ | grep sur_refl_b02 | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			 #Update B02@250m file
			 f2=$(find -type f | grep MOD09Q1_ | grep sur_refl_b02 | grep A$doy | grep .tif)
			 #count GeoTiff identified
			 c2=$(find -type f | grep MOD09Q1_ | grep sur_refl_b02 | grep A$doy | grep .tif | wc -l)
		fi
		if [ $(find -type f | grep MOD09Q1_ | grep sur_refl_qc_250m | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			 #Update BQA@250m file
			 f4=$(find -type f | grep MOD09Q1_ | grep sur_refl_qc_250m | grep A$doy | grep .tif)
			 #count GeoTiff identified
			 c4=$(find -type f | grep MOD09Q1_ | grep sur_refl_qc_250m | grep A$doy | grep .tif | wc -l)
		fi
		if [ $(find -type f | grep MOD09A1_ | grep sur_refl_b07 | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			 #Update B07@500m file
			 f3=$(find -type f | grep MOD09A1_ | grep sur_refl_b07 | grep A$doy | grep .tif)
			 #count GeoTiff identified
			 c3=$(find -type f | grep MOD09A1_ | grep sur_refl_b07 | grep A$doy | grep .tif | wc -l)
		fi
				if [ $(find -type f | grep MOD09A1_ | grep sur_refl_qc | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			 #Update BQA@500m file
			 f5=$(find -type f | grep MOD09A1_ | grep sur_refl_qc | grep A$doy | grep .tif)
			 #count GeoTiff identified
			 c5=$(find -type f | grep MOD09A1_ | grep sur_refl_qc | grep A$doy | grep .tif | wc -l)
		fi
		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $c5 == 1 ]]
		then
			#Define number of gdalwarp running
			npid=$(echo "$(ps aux | grep r_net | wc -l) - 1" | bc)
			while [ $npid -ge $(echo $ncores "*4" | bc) ]
			do
				sleep 1
				#Update number of lst running
				npid=$(echo "$(ps aux | grep r_net | wc -l) - 1" | bc)
				#Update number of (virtual) cores (for heterogeneous systems)
				#ncores=`grep -c 'processor' /proc/cpuinfo`
			done
			echo -e "\e[01;33m" "wm" $f1 $f2 $f3 $f4 $f5 $f6"\e[00m"
			f6=~/DATA/water_$doy\_250m.tif
			#process
			#$prog_root/wm $inB1 $inB2 $inB7 $in250QC $in500QC $outWATER &
			$prog_root/wm $f1 $f2 $f3 $f4 $f5 $f6 &
		fi
		
	done
done
