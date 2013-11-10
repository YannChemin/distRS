#!/bin/bash
#Please find those details
full_area_name=CIAF0309
subset_name=cia

img_base_name=theta_sebal_
# original data from ETb
img_root=/media/disk2part1/RS/3_Products/theta_sebal/
# img_root=eta/

#Should not need to modify after this
outdir=$subset_name
mkdir $outdir
echo $outdir
rm $outdir/*.tif -f

ogrinfo $full_area_name.shp $full_area_name | grep Extent > tempfile.txt
box=$(sed 's/Extent: (\(.*\), \(.*\)) - (\(.*\), \(.*\))/ \1\ \2\ \3\ \4/' tempfile.txt)

for file in $img_root/$img_base_name*.tif
do
	echo $file > tempfile.txt
	awk -F[_] '{ print $NF }' tempfile.txt > out.txt
	outfile=$(cat out.txt)
	echo $outdir/$subset_name\_theta_$outfile
	gdalwarp -q -te $box -cutline $full_area_name.shp $file $outdir/$subset_name\_theta_$outfile
done 

