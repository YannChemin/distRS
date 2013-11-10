#!/bin/bash

root=../processedgtiff
cd $root

for hv in h27v07 h28v07
do
	for band in b01_250m b01_500m b02_250m b02_500m b03_500m b04_500m b05_500m b06_500m b07_500m qc_250m state_500m
	do
		files=$(ls *$hv\_$band*)
		../sum/sum ../sum/outSUM$hv$band $files
	done
done