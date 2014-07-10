#!/bin/bash

#Code can open 15000 files
#But Linux by default can open 1024 only

#if "Too many open files" error
#sudo su
#vim /etc/security/limits.conf
# # Limit user nofile - max number of open files
# * soft  nofile 15000
# * hard  nofile 18000
#Then restart the user session (i.e., logout, login). 

#Go to Data Directory
cd ~/DATA

PRoot=~/dev/distRS/trunk

#FREQUENCY PROCESSING
prog_root=$PRoot/prog/prog_freq
cd $prog_root
make clean
make

cd ~/DATA
files=$(ls water_*.tif)

outFREQ=~/DATA/water_freq.tif

$prog_root/freq $outFREQ $files  
