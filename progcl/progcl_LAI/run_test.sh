#!/bin/bash
# ./ndvi fndvi.tif fb3.tif fb4.tif kernels.cl


laidoySTART=2002184
laidoyEND=2013000
#DEM PROCESSING
#dump all srtm tiles you downloaded into the srtm directory
#then set your geographical box of interest (tiles have to cover it)
# xmin=79.25
# ymin=5.8
# xmax=82.07
# ymax=9.967
# WGS84='EPSG:4326'
#Go to Data Directory and fetch the first raster file resolution
# cd /media/OneTBFive/LK
# file=$(ls | head -1)
# tar xvf $file
# xpix=$(gdalinfo $(echo $file | sed 's/.tar.gz//g') | grep Pixel | sed 's/Pixel\ Size\ =\ (\(.*\),\(.*\))/\1/g')
# ypix=$(gdalinfo $(echo $file | sed 's/.tar.gz//g') | grep Pixel | sed 's/Pixel\ Size\ =\ (\(.*\),-\(.*\))/\2/g')
#Go to SRTM Data Directory
# cd /media/OneTBFive/srtm
#merge tiles and reproject
# rm -f srtm.tif
# gdalwarp -of GTiff -ot Float32 -t_srs $WGS84 -te $xmin $ymin $xmax $ymax -tr $xpix $ypix $(ls srtm_*.tif) srtm.tif

#Go to Data Directory
cd /home/yann/Coding/progcl/prog_LAI/input
rm -f *.xml

#LAI PROCESSING
laiprog_root=/home/yann/Coding/progcl/prog_LAI
mkdir -p $laiprog_root/output/
rm -f $laiprog_root/output/*

for (( laidoy = $laidoySTART ; laidoy <= $laidoyEND ; laidoy ++ ))
do
	if [ $(expr $laidoy % 1000) -lt 366 ]
	then
		echo "LAI" $laidoy
		#count tarball identified
		laic1=$(find -type f | grep MCD15A3  | grep A$laidoy | grep Lai_1km.tif | grep .tar.gz | wc -l)
#		if [ $laic1 -eq 1 -a $laic2 -eq 1 -a $laioutno -eq 0 ]
		if [ $laic1 -eq 1 ]
		then
			#count tarball identified
			laic2=$(find -type f | grep MCD15A3  | grep A$laidoy | grep FparLai_QC.tif | grep .tar.gz | wc -l)
			if [ $laic2 -eq 1 ]
			then
				#find tarball
				lait1=$(find -type f | grep MCD15A3  | grep A$laidoy | grep .tar.gz | grep Lai_1km.tif)
				#extract filename
				laif1=$(echo $lait1 | sed 's/.tar.gz//g')
				#QA file
				#find tarball
				lait2=$(find -type f | grep MCD15A3  | grep A$laidoy | grep .tar.gz | grep FparLai_QC.tif)
				#extract filename
				laif2=$(echo $lait2 | sed 's/.tar.gz//g')

				#Output filename
				laiout=$laiprog_root/output/lai_$laidoy.tif
				#does it already exist?
				laioutno=$(find -type f | grep $laiout | wc -l)

				echo -e "\e[01;36m" "lai" $laif1 $laif2 $laiout "\e[00m"
				#extract tarballs
				tar xvf $lait1
				tar xvf $lait2
				#process
				$laiprog_root/modis_lai_clean_qa $laiprog_root/kernel.cl $laiout $laif1 $laif2
			fi
		fi
	fi
done

