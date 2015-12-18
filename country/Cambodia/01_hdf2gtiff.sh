#!/bin/bash
Root=/home/ubuntu/ladsweb.nascom.nasa.gov/allData/5/MOD11A1
xmin=102.0
ymin=10.0
xmax=106.9
ymax=15.25
outRoot=/home/ubuntu/LST
mkdir -p $outRoot
for file in $outRoot/HDF4*.tif
do
	rm -f $file&
done

WGS84='EPSG:4326'

#Define number of (virtual) cores
ncores=`grep -c 'core id' /proc/cpuinfo`
echo "ncores=" $ncores

for location in "$Root/2000/" "$Root/2001/" "$Root/2002/" "$Root/2003/"  "$Root/2004/" "$Root/2005/" "$Root/2006/" "$Root/2007/" "$Root/2008/" "$Root/2009/" "$Root/2010/" "$Root/2011/" "$Root/2012/" "$Root/2013/" "$Root/2014/"
do
	cd $location
	for day in $(ls -d *)
	do
		#Dive into the directory of the date of overpass
		cd $day
		echo $day
		rm -f *.xml
		file=$(ls *h28v07*.hdf)
		SDS=$(gdalinfo $file | grep "SUBDATASET_1_NAME")
		out=$(echo ${SDS#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		#echo "OUT=" $out
		#Define number of gdalwarp running
		npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
		while [ $npid -ge $ncores ]
		do
			sleep 1
			#Update number of gdalwarp running
			npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
			#Update number of (virtual) cores
			ncores=`grep -c 'core id' /proc/cpuinfo`
		done
		#Process gdalwarp with two SDS tiles
		gdalwarp -of GTiff -q -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $WGS84 -te $xmin $ymin $xmax $ymax "${SDS#*=}" $outRoot/$out.tif &
		cd ..
	done
	cd ..
done

for location in "$Root/2000/" "$Root/2001/" "$Root/2002/" "$Root/2003/"  "$Root/2004/" "$Root/2005/" "$Root/2006/" "$Root/2007/" "$Root/2008/" "$Root/2009/" "$Root/2010/" "$Root/2011/" "$Root/2012/" "$Root/2013/" "$Root/2014/"
do
	cd $location
	for day in $(ls -d *)
	do
		#Dive into the directory of the date of overpass
		cd $day
		echo $day
		rm -f *.xml
		file=$(ls MOD11A1*h28v07*.hdf)
		if [ $file ]
		then
		SDS=$(gdalinfo $file | grep "SUBDATASET_2_NAME")
		out=$(echo ${SDS#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		#echo "OUT=" $out
		#Define number of gdalwarp running
		npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
		while [ $npid -ge $ncores ]
		do
			sleep 1
			#Update number of gdalwarp running
			npid=$(echo "$(ps aux | grep gdalwarp | wc -l) - 1" | bc)
			#Update number of (virtual) cores
			ncores=`grep -c 'core id' /proc/cpuinfo`
		done
		#Process gdalwarp with two SDS tiles
		gdalwarp -of GTiff -q -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $WGS84 -te $xmin $ymin $xmax $ymax "${SDS#*=}" $outRoot/$out.tif &
		fi
		cd ..
	done
	cd ..
done

