#!/bin/bash

tgzF=$1

if [ $# -lt 1 ]; then
  echo 1>&2 "$0: not enough arguments: TGZ input file"
  exit 2
elif [ $# -gt 1 ]; then
  echo 1>&2 "$0: too many arguments"
  exit 2
fi

#INPUT L8 DIR
in_l8=~/RSDATA/IN_L8/

#Make output directory
out_l8=~/RSDATA/OUT_L8
mkdir -p $out_l8

# Recompile with latest changes
make clean &>/dev/null
make &>/dev/null


#pwd of program
prog=~/L8T1Qa/trunk/prog_L8T1Qa/

#CD to input dir
cd $in_l8

# Uncompress tarball
mkdir -p $in_l8/$(echo $1 | sed 's/.tar.gz//')
tar -I pigz -xvf $tgzF -C $in_l8/$(echo $1 | sed 's/.tar.gz//') &>/dev/null

# Go to untarballed dir
cd $in_l8/$(echo $tgzF | sed 's/.tar.gz//')

# Set input names
inB4=$(ls *band4.tif)
inB5=$(ls *band5.tif)
inB6=$(ls *band6.tif)
inB7=$(ls *band7.tif)
inBQ=$(ls *pixel_qa.tif)

# Set output names
outL8VI=$out_l8/$(echo $tgzF | sed 's/.tar.gz//')\_NDVI.tif
outL8WI=$out_l8/$(echo $tgzF | sed 's/.tar.gz//')\_NDWI.tif
outL8LSWI=$out_l8/$(echo $tgzF | sed 's/.tar.gz//')\_LSWI.tif
outL8NBR2=$out_l8/$(echo $tgzF | sed 's/.tar.gz//')\_NBR2.tif
	
# Set temp names
tmpL8VI=$out_l8/temp_$(echo $tgzF | sed 's/.tar.gz//')\_NDVI.tif
tmpL8WI=$out_l8/temp_$(echo $tgzF | sed 's/.tar.gz//')\_NDWI.tif
tmpL8LSWI=$out_l8/temp_$(echo $tgzF | sed 's/.tar.gz//')\_LSWI.tif
tmpL8NBR2=$out_l8/temp_$(echo $tgzF | sed 's/.tar.gz//')\_NBR2.tif
	
# Process
#echo "$prog/l8t1qa $inB4 $inB5 $inB6 $inB7 $inBQ $tmpL8VI $tmpL8WI $tmpL8LSWI $tmpL8NBR2" 
$prog/l8t1qa $inB4 $inB5 $inB6 $inB7 $inBQ $tmpL8VI $tmpL8WI $tmpL8LSWI $tmpL8NBR2
#echo "python ./run.py $inB4 $inB5 $inB6 $inB7 $inBQ $tmpL8VI $tmpL8WI $tmpL8LSWI $tmpL8NBR2" 
#python ./run.py $inB4 $inB5 $inB6 $inB7 $inBQ $tmpL8VI $tmpL8WI $tmpL8LSWI $tmpL8NBR2 

# Clean up
cd $in_l8
rm -Rf $in_l8/$(echo $tgzF | sed 's/.tar.gz//')

# Go to output Dir
cd $out_l8
#echo "cd $out_l8"

# Convert tmp to EPSG:4326 & clean up
gdalwarp -q -multi -t_srs "EPSG:4326" $tmpL8VI $outL8VI
gdalwarp -q -multi -t_srs "EPSG:4326" $tmpL8WI $outL8WI
gdalwarp -q -multi -t_srs "EPSG:4326" $tmpL8LSWI $outL8LSWI
gdalwarp -q -multi -t_srs "EPSG:4326" $tmpL8NBR2 $outL8NBR2
rm -f $tmpL8VI &
rm -f $tmpL8WI &
rm -f $tmpL8LSWI &
rm -f $tmpL8NBR2 &

# Tarball the output file & clean up
tar -I pigz -cvf $(echo $outL8VI | sed 's/.tif//').tar.gz --directory=$out_l8 $(echo $tgzF | sed 's/.tar.gz//')\_NDVI.tif &>/dev/null
tar -I pigz -cvf $(echo $outL8WI | sed 's/.tif//').tar.gz --directory=$out_l8 $(echo $tgzF | sed 's/.tar.gz//')\_NDWI.tif &>/dev/null
tar -I pigz -cvf $(echo $outL8LSWI | sed 's/.tif//').tar.gz --directory=$out_l8 $(echo $tgzF | sed 's/.tar.gz//')\_LSWI.tif &>/dev/null
tar -I pigz -cvf $(echo $outL8NBR2 | sed 's/.tif//').tar.gz --directory=$out_l8 $(echo $tgzF | sed 's/.tar.gz//')\_NBR2.tif &>/dev/null
rm -f $outL8VI &
rm -f $outL8WI &
rm -f $outL8LSWI &
rm -f $outL8NBR2 &
rm -f *.IMD
