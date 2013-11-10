#!/bin/bash
root=$(pwd)
in=$root/processed/
out=$root/ef/

cd $in
for file in ef*.tif
do
	outfile=$(echo $file | sed 's/ef_\(.*\)_A\(.*\)_005\(.*\).tif/ef_\2\.tif/g') 
	echo $outfile
	gdalwarp -cutline $root/shp/Hakra_Command.shp $file $out/$outfile 
done
