#!/bin/bash
# ./ndvi fndvi.tif fb3.tif fb4.tif kernels.cl


doySTART=2000048
doyEND=2013000
#DEM PROCESSING
#dump all srtm tiles you downloaded into the srtm directory
#then set your geographical box of interest (tiles have to cover it)
xmin=79.25
ymin=5.8
xmax=82.07
ymax=9.967
WGS84='EPSG:4326'
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
cd /media/OneTBFive/LK

# #NDVI PROCESSING
# prog_root=/home/yann/Coding/progcl/prog_NDVI
#
# for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
# do
# 	if [ $(expr $doy % 1000) -lt 366 ]
# 	then
# 		echo "ndvi" $doy
# 		#count tarball identified
# 		c1=$(find -type f | grep MOD13A2  | grep A$doy | grep NDVI | grep .tar.gz | wc -l)
# 		if [ $c1 -eq 1 ]
# 		then
# 			#count tarball identified
# 			c2=$(find -type f | grep MOD13A2  | grep A$doy | grep reliability | grep .tar.gz | wc -l)
# 	#		if [ $c1 -eq 1 -a $c2 -eq 1 -a $outno -eq 0 ]
# 			if [ $c2 -eq 1 ]
# 			then
# 				#NDVI file
# 				#find tarball
# 				t1=$(find -type f | grep MOD13A2  | grep A$doy | grep NDVI | grep .tar.gz)
# 				#extract filename
# 				f1=$(echo $t1 | sed 's/.tar.gz//g')
# 				#QA file
# 				#find tarball
# 				t2=$(find -type f | grep MOD13A2  | grep A$doy | grep reliability | grep .tar.gz)
# 				#extract filename
# 				f2=$(echo $t2 | sed 's/.tar.gz//g')
#
# 				#Output filename
# 				out=ndvi_$doy.tif
# 				#does it already exist?
# 				outno=$(find -type f | grep $out | wc -l)
#
# 				echo -e "\e[01;36m" "ndvi" $f1 $f2 $out "\e[00m"
# 				#extract tarballs
# 				tar xvf $t1
# 				tar xvf $t2
# 				sleep 1
# 				#process
# 				$prog_root/modis_ndvi_clean_qa $prog_root/kernel.cl $out $f1 $f2 &
# 			fi
# 		fi
# 	fi
# done

#LAI PROCESSING
prog_root=/home/yann/Coding/progcl/prog_LAI

for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "LAI" $doy
		#count tarball identified
		c1=$(find -type f | grep MCD15A3  | grep A$doy | grep Lai_1km.tif | grep .tar.gz | wc -l)
#		if [ $c1 -eq 1 -a $c2 -eq 1 -a $outno -eq 0 ]
		if [ $c1 -eq 1 ]
		then
			#count tarball identified
			c2=$(find -type f | grep MCD15A3  | grep A$doy | grep FparLai_QC.tif | grep .tar.gz | wc -l)
			if [ $c2 -eq 1 ]
			then
				#find tarball
				t1=$(find -type f | grep MCD15A3  | grep A$doy | grep .tar.gz | grep Lai_1km.tif)
				#extract filename
				f1=$(echo $t1 | sed 's/.tar.gz//g')
				#QA file
				#find tarball
				t2=$(find -type f | grep MCD15A3  | grep A$doy | grep .tar.gz | grep FparLai_QC.tif)
				#extract filename
				f2=$(echo $t2 | sed 's/.tar.gz//g')

				#Output filename
				out=lai_$doy.tif
				#does it already exist?
				outno=$(find -type f | grep $out | wc -l)

				echo -e "\e[01;36m" "lai" $f1 $f2 $out "\e[00m"
				#extract tarballs
				tar xvf $t1
				tar xvf $t2
				sleep 1
				#process
				$prog_root/modis_lai_clean_qa $prog_root/kernel.cl $out $f1 $f2 &
			fi
		fi
	fi
done

