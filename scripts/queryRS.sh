#!/bin/bash

# $1 : Source file (Raster image)
# $2 : Longitude coordinate of the Raster image (North-South)
# $3 : Latitude coordinate of the Raster image (East-West)

#Process search for pixel values
echo $(gdallocationinfo -valonly -wgs84 $1 $2 $3)
