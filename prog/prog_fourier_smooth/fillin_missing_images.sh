#!/bin/bash

#23 dates per year
listDOY="001 017 033 049 065 081 097 113 129 145 161 177 193 209 225 241 257 273 289 305 321 337 353"

root=cia_NDVI_2000_2010
pre=cia_NDVI_
suf=.tif
dummyFILEsrc=$pre\2000049$suf

for (( year = 2000 ; year < 2012 ; year++ ))
do
	for doy in $listDOY
	do
		test=$(find $root/$pre$year$doy$suf -type f | wc -l)
		if [ $test0 -eq 0 ]
			then echo "replacing missing file with dummy:" $pre$year$doy$suf
			./createNodataRaster -32768 $dummyFILEsrc $pre$year$doy$suf
		fi
	done
done