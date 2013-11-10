#!/bin/bash
root=$(pwd)
in=$root/ef/

rm tempfile.txt -f
pre=ef_
echo $pre
for (( doy = 2000000 ; doy < 2012000 ; doy++ ))
	do test1=$(find $in -type f | grep $pre$doy | wc -l)
	if [ $test1 -eq 1 ]
	then 
	# 	echo "./stats" $in/$pre$doy".tif -28768"
		$root/prog_STATS2/stats $in/$pre$doy.tif 0 >> tempfile.txt
	fi
done

mv tempfile.txt Hakra_stats.csv
