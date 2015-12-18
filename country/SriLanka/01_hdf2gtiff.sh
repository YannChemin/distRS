#!/bin/bash
Root=~/ladsweb.nascom.nasa.gov/allData/5/MOD11A1
xmin=79.4
ymin=5.9
xmax=82.0
ymax=9.9
outRoot=~/DATA
#mkdir -p $outRoot
#for file in $outRoot/HDF4*.tif
#do
#	rm -f $file&
#done

WGS84='EPSG:4326'

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores

for location in "$Root/2000/" "$Root/2001/" "$Root/2002/" "$Root/2003/" "$Root/2004/" "$Root/2005/" "$Root/2006/" "$Root/2007/" "$Root/2008/" "$Root/2009/" "$Root/2010/" "$Root/2011/" "$Root/2012/" "$Root/2013/" "$Root/2014/" "$Root/2015/"
do
	cd $location
	for day in $(ls -d *)
	do
		#Dive into the directory of the date of overpass
		cd $day
		echo $location "/" $day
		rm -f *.xml
		if [ $( find . -name "MOD11A1*h25v08*.hdf" | wc -l ) -eq 1 ]
		then
			file=$(ls MOD11A1*h25v08*.hdf)
			SDS0=$(gdalinfo $file | grep "SUBDATASET_1_NAME")
			file=$(ls MOD11A1*h26v08*.hdf)
			SDS1=$(gdalinfo $file | grep "SUBDATASET_1_NAME")
			out=$(echo ${SDS0#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
			echo "OUT=" $out
			#Define number of gdalwarp running
			npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
			while [ $npid -ge $ncores ]
			do
				sleep 1
				#Update number of gdalwarp running
				npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
				#Update number of (virtual) cores
				ncores=`grep -c 'processor' /proc/cpuinfo`
			done
			#Process gdalwarp with two SDS tiles
			#echo "gdalwarp -of GTiff -q -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $WGS84 -te $xmin $ymin $xmax $ymax "${SDS#*=}" $outRoot/$out.tif &"
			gdalwarp -of GTiff -q -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $WGS84 -te $xmin $ymin $xmax $ymax "${SDS0#*=}" "${SDS1#*=}" $outRoot/$out.tif &
		fi
		cd ..
	done
	cd ..
done


