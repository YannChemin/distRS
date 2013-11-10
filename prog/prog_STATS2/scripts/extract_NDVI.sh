#!/bin/bash
#Please find those details
full_area_name=CIAF0309
subset_name=cia

img_base_name=MOD13A2_
# original data from ETb
img_root=/media/disk2part1/RS/3_Products/NDVI/

#Should not need to modify after this
outdir=$(pwd)/$subset_name
mkdir $outdir
echo $outdir
rm $outdir/*NDVI*.tif -f

ogrinfo $full_area_name.shp $full_area_name | grep Extent > tempfile.txt
box=$(sed 's/Extent: (\(.*\), \(.*\)) - (\(.*\), \(.*\))/ \1\ \2\ \3\ \4/' tempfile.txt)

cd $img_root

for file in $img_base_name*.tif
do
	echo "file ="$file
	outfile=$(echo $file | sed 's/MOD13A2_//g' | sed 's/_1Km_16D_NDVI.tif//g' )
	echo $outdir/$subset_name\_NDVI_$outfile.tif
	gdalwarp -q -te $box -cutline $outdir/../$full_area_name.shp $file $outdir/$subset_name\_NDVI_$outfile.tif
done 

