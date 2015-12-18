#!/bin/bash
doySTART=2000000
doyEND=2001000
root=~/dev/distRS/trunk/prog/prog_STATS2
cd $root
make

cp -f ~/DATA/ta_int/ta_*.tif ~/DATA/
cp -f ~/DATA/ta_int/eta_sebal_*.tif ~/DATA/

#Go to Data Directory
cd ~/DATA

#compile statistics in other directory
output=~/DATA/stats
mkdir -p $output
rm -f $output/*

for prefix in eta_sebal eta_sseb eto_pm et_potd ta
do
	echo "file,count,sum,min,max,average,stdev" > $output/$prefix.csv
	for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
	do
		if [ $(expr $doy % 1000) -lt 366 ]
		then
			echo $prefix $doy
			test1=$(find -type f | grep ./$prefix | grep $doy | grep .tif | wc -l)
			if [ $test1 -eq 1 ]
			then
				file1=$(find -type f | grep ./$prefix | grep $doy | grep .tif)
				$root/stats $file1 0 >> $output/$prefix.csv
			else
				echo "$doy.tif,,,,,," >> $output/$prefix.csv
			fi
		fi
	done
done

for prefix in evapfr_sebal evapfr_sseb
do
	echo "file,count,sum,min,max,average,stdev" > $output/$prefix.csv
	for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
	do
		if [ $(expr $doy % 1000) -lt 366 ]
		then
			echo $prefix $doy
			test1=$(find -type f | grep ./$prefix | grep $doy | grep .tif | wc -l)
			if [ $test1 -eq 1 ]
			then
				file1=$(find -type f | grep ./$prefix | grep $doy | grep .tif)
				$root/stats $file1 0 >> $output/$prefix.csv
			else
				echo "$doy.tif,,,,,," >> $output/$prefix.csv
			fi
		fi
	done
done

# rm *tif -f

cd $output
# Make csv files readable for Python Matplotlib graphing
for file1 in *.csv
do
	sed 's/.tif//g' $file1 | sed 's/\.\///g' | sed 's/et_potd_//g' | sed 's/eto_pm_//g' | sed 's/eta_sseb_//g' |sed 's/ta_//g' | sed 's/eta_sebal_//g' | sed 's/evapfr_sebal_//g' |sed 's/evapfr_metric_//g' |sed 's/evapfr_sseb_//g' |   sed 's/Input\ file\ could\ not\ be\ loaded//g' | sed 's/0,0.000000,100000.000000,0.000000,,/,,,,,/g'| sed 's/100000.000000//g'|sed 's/-nan//g'| sed '/^$/d' > temp.txt
	mv temp.txt $file1
done
