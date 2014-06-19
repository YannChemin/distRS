#!/bin/bash
root=$(pwd)
#images root
imgRoot=$root/processed
#echo "image root is $imgRoot"
#shapefiles root
shpRoot=$root/shp
#echo "shp root is $shpRoot"

ncores=$(grep -e cpuid /proc/cpuinfo | wc -l)



#Look for the shapefiles
cd $shpRoot
for shapefile in *.shp
do
	cd $shpRoot
	echo $shapefile
	feature_count=$(ogrinfo -al $shapefile | grep "Feature Count" | sed 's/Feature\ Count:\ //g')
	echo "feature count=" $feature_count
	count=0
	while [ $count -lt $feature_count ]
	do
		echo "polygon" $count
		dir=$(echo $shapefile | sed 's/shp//g')$count
		echo $dir
		mkdir -p $shpRoot/$dir
		cd $imgRoot
		for image in *.tif
		do
			rm -f $shpRoot/$dir/$image
			#echo "$imgRoot/$image"
			#echo "$shpRoot/$shapefile"
			while [ $(ps aux | grep gdalwarp | wc -l) -ge $ncores ]
			do
				sleep 5
			done
			if [ $count -gt 1 ]
			then
				gdalwarp -q -cutline $shpRoot/$shapefile -cl $count -crop_to_cutline $imgRoot/$image $shpRoot/$dir/$image &
			else
				gdalwarp -q -cutline $shpRoot/$shapefile -crop_to_cutline $imgRoot/$image $shpRoot/$dir/$image &
			fi
		done
		count=$(echo "$count + 1" | bc)
	done
done

