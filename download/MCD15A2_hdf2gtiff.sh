#!/bin/bash
#cutline is here 
cutlineV='/data/GR/Greece_Rice_5_SimUnits.shp'
#no data value
NODATA=255
Root=/data/mcd15a2h
xmin=5374759
ymin=2016255
xmax=5450835
ymax=2117293
outRoot=/data/GR/mcd15a2h
mkdir -p $outRoot
for file in $outRoot/HDF4*.tif
do
	rm -f $file&
done
LAEA='EPSG:3035'
WGS84='EPSG:4326'

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores
#Dive into the directory of the date of overpass
for year in 2001 2002 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014 2015 2016 2017 2018 2019
do
	for doy in 001 009 017 025 033 041 049 057 065 073 081 089 097 105 113 121 129 137 145 153 161 177 185 193 201 209 217 225 233 241 249 257 265 273 281 289 297 305 313 321 329 337 345 353 361 
	do
		echo $year$doy
		file=$(ls MCD15A2H.A$year$doy.h19v04*.hdf)
		SDS11=$(gdalinfo $file | grep "SUBDATASET_1_NAME")
		out=$(echo ${SDS11#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		echo "OUT=" $out
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATA -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS11#*=}" $outRoot/$out.tif &
		file=$(ls MCD15A2H.A$year$doy.h19v04*.hdf)
		SDS11=$(gdalinfo $file | grep "SUBDATASET_2_NAME")
		out=$(echo ${SDS11#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		echo "OUT=" $out
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATA -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS11#*=}" $outRoot/$out.tif &
		file=$(ls MCD15A2H.A$year$doy.h19v04*.hdf)
		SDS11=$(gdalinfo $file | grep "SUBDATASET_3_NAME")
		out=$(echo ${SDS11#*=} | sed 's/[\/\"\:\.\ ]/\_/g' | sed 's/\(.*\)_h..v.._\(.*\)/\1\_\2/g')
		echo "OUT=" $out
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATA -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS11#*=}" $outRoot/$out.tif &
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
		rm -f $outRoot/$out.tif
		gdalwarp -of GTiff -q -cutline $cutlineV -crop_to_cutline -dstnodata $NODATA -s_srs '+proj=sinu +R=6371007.181 +nadgrids=@null +wktext' -t_srs $LAEA -te $xmin $ymin $xmax $ymax "${SDS11#*=}" $outRoot/$out.tif 
	done
done
