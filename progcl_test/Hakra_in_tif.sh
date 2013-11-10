#!/bin/bash

#set Hakra lat/lon
top=29.96
bottom=29.05
left=72.25
right=73.4

ulx=$left
uly=$top
lrx=$right
lry=$bottom

root=$(pwd)
working_root=/media/disk4part1/e4ftl01u.ecs.nasa.gov/MOLT/MOD11A1.005/processed/

cd $working_root
for file in *LST_LST*.tar.gz
do
	tar xvf $file
	working_file=$(ls *LST_LST*.tif)
	#Do a loop in case more than one file are around
	for wfile in $working_file
	do
		gdal_translate -ot Float32 -projwin $ulx $uly $lrx $lry $wfile $root/$wfile
		rm -f $wfile
	done
done
