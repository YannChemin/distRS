#!/bin/bash
#Please find those details
full_area_name=CIAF0309
subset_name=cia

img_base_name=MOD11A1_
# original data from ETb
img_root=/media/disk2part1/RS/3_Products/LST/

#Should not need to modify after this
outdir=$(pwd)/$subset_name
mkdir $outdir
echo $outdir
rm $outdir/*LST*.tif -f

ogrinfo $full_area_name.shp $full_area_name | grep Extent > tempfile.txt
box=$(sed 's/Extent: (\(.*\), \(.*\)) - (\(.*\), \(.*\))/ \1\ \2\ \3\ \4/' tempfile.txt)

cd $img_root

for file in $img_base_name*.tif
do
	echo "file ="$file
	outfile=$(echo $file | sed 's/MOD11A1_//g' | sed 's/_1Km_1D_LST.tif//g' )
	echo $outdir/$subset_name\_LST_$outfile.tif
	gdalwarp -q -te $box -cutline $outdir/../$full_area_name.shp $file $outdir/$subset_name\_LST_$outfile.tif
done 

