#!/bin/bash

progroot=/home/yann/dev/distRS/trunk/prog/prog_avg/
cd $progroot
make clean
make

outroot=/media/yann/Data/TonleSapTaGap/yearly
mkdir -p $outroot
dataroot=/home/yann/TonleSapTaGap
cd $dataroot


for year in 2000 2001 2002 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014
do files=$(ls ta_gap_*$year*.tif)
$progroot/sum $outroot/avg_ta_gap_$year.tif $files
done






#for hv in h27v07 h28v07
#do
#	for band in b01_250m b01_500m b02_250m b02_500m b03_500m b04_500m b05_500m b06_500m b07_500m qc_250m state_500m
#	do
#		files=$(ls *$hv\_$band*)
#		../sum/sum ../sum/outSUM$hv$band $files
#	done
#done
