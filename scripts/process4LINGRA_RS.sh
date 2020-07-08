#!/bin/bash

#Run this from the RSDATA/ dir
RSDTRoot=$PWD

#Clean & Make 
progRoot=$HOME/dev/distRS/trunk/prog
cd $progRoot/prog_LAI
make clean
make
cd $RSDTRoot

#PROCESS LAI
for file in MCD15A2H*
do
	LAIF=$(gdalinfo $file | grep SUBDATASET_2_NAME | sed "s/\ \ SUBDATASET_2_NAME=//")
	LAIOUT=$(echo $LAIF | sed 's/\(.*\)\"\(.*\)\"\(.*\)/\2/' | sed 's/.hdf/_LAI.tif/')
	if [ -f "$LAIOUT" ]; then
    	echo "$LAIOUT exists, Not Overwriting."
	else
		echo $LAIF
		Offset=$(gdalinfo $LAIF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\1/")
		Scale=$(gdalinfo $LAIF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\2/") 
		LAIFQC=$(gdalinfo $file | grep SUBDATASET_3_NAME | sed "s/\ \ SUBDATASET_3_NAME=//")
		$progRoot/prog_LAI/lai $LAIF $LAIFQC $LAIOUT $Offset $Scale
	fi
done

#PROCESS MOD ETA
for file in MOD16A2*
do
	ETAF=$(gdalinfo $file | grep SUBDATASET_1_NAME | sed 's/\ \ SUBDATASET_1_NAME=//')
	ETAOUT=$(echo $ETAF | sed 's/\(.*\)\"\(.*\)\"\(.*\)/\2/' | sed 's/.hdf/_ETA.tif/')
	if [ -f "$ETAOUT" ]; then
    	echo "$ETAOUT exists, Not Overwriting."
	else
		echo $ETAF
		Offset=$(gdalinfo $ETAF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\1/")
		Scale=$(gdalinfo $ETAF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\2/") 
		ETAFQC=$(gdalinfo $file | grep SUBDATASET_5_NAME | sed 's/\ \ SUBDATASET_5_NAME=//')
		$progRoot/prog_LAI/lai $ETAF $ETAFQC $ETAOUT $Offset $Scale
	fi
done
	
#PROCESS MYD ETA
for file in MYD16A2*
do
	ETAF=$(gdalinfo $file | grep SUBDATASET_1_NAME | sed 's/\ \ SUBDATASET_1_NAME=//')
	ETAOUT=$(echo $ETAF | sed 's/\(.*\)\"\(.*\)\"\(.*\)/\2/' | sed 's/.hdf/_ETA.tif/')
	if [ -f "$ETAOUT" ]; then
    	echo "$ETAOUT exists, Not Overwriting."
	else
		echo $ETAF
		Offset=$(gdalinfo $ETAF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\1/")
		Scale=$(gdalinfo $ETAF | grep Offset:\ | sed "s/\ \ Offset:\ \(.*\),\ \ \ Scale:\(.*\)/\2/") 
		ETAFQC=$(gdalinfo $file | grep SUBDATASET_5_NAME | sed 's/\ \ SUBDATASET_5_NAME=//')
		$progRoot/prog_LAI/lai $ETAF $ETAFQC $ETAOUT $Offset $Scale
	fi
done
	
