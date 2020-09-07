#!/bin/bash

# Query A Single Pixel in all images
#------------------------------------------------------------
# Arguments: bash queryRSAll.sh $1 $2 $3 $4
#------------------------------------------------------------
# $1 : Source directory (with many Raster images i.e. ~/RSDATA/)
# $2 : Longitude coordinate of the Raster image (East-West)
# $3 : Latitude coordinate of the Raster image (North-South)
# $4 : One of the Indices: NDVI NDWI LSWI NBR2
#------------------------------------------------------------
# Returns a set of pixel values in STDOUT
# can be caught by Python subprocess()
#------------------------------------------------------------

# Identify the Index
VI=$4

# Process search for pixel values
cd $1

# Get Path and Row to examine
pathRow=$(Hco_RS_getPathRow $2 $3)

for file in $(ls LC08$pathRow*$VI.tif)
do
	dateT=$(echo $file | sed 's/LC08......\(.*\)..T.-\(.*\)/\1/')
	if [[ $(gdallocationinfo -valonly -wgs84 $file $2 $3) -ne "" ]]
	then
		echo $dateT","$(gdallocationinfo -valonly -wgs84 $file $2 $3) 
	fi
done

