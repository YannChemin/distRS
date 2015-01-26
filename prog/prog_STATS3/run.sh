#!/bin/bash
root=$(pwd)
echo "root=$root"
in=$root/shp
echo "in=$in"
rm $root/output/*tempfile.csv -f

cd $in
in=$(pwd)
echo "pwd=$(pwd)"
#for dir in $(ls -d $in)
#for dir in $(ls */ -d | sed 's/\(.*\)\.\(.*\)\//\1.\2/g') 
for dir in $(ls */ -d) 
do
	cd $dir
	touch $root/output/$dir\.tempfile.csv
	echo "loop=$dir"
	for file in *.tif
	do 
		echo $file
		echo "$root/stats $in/$dir/$file 0"
		$root/stats $file 0 >> $root/output/$(echo $dir | sed "s/\///g")\.tempfile.csv
	done
	cd ..
done
