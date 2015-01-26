#!/bin/bash

root=~/DATASL
cd $root

for hv in h25v08 h26v08
do
	files=$(ls water_$hv\_*.tif)
	~/dev/distRS/trunk/prog/prog_avg/avg $root/water_avg_$hv\.tif $files
done

