#!/bin/bash
doySTART=2000000
doyEND=2016000
#Merge datasets
#mkdir -p ~/DATA
#DEM PROCESSING
#dump all srtm tiles you downloaded into the srtm directory
#then set your geographical box of interest (tiles have to cover it)
xmin=79.4
ymin=5.9
xmax=82.0
ymax=9.9
WGS84='EPSG:4326'
#Go to Data Directory and fetch the first raster file resolution
cd ~/DATA
PRoot=~/dev/distRS/trunk

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores

#NDVI PROCESSING
prog_root=$PRoot/prog/prog_NDVI
cd $prog_root
make clean
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "ndvi" $doy
		#count tarball identified
		c1=$(find -type f | grep MOD13A2  | grep A$doy | grep NDVI | wc -l)
		if [ $c1 -eq 1 ]
		then
			#count tarball identified
			c2=$(find -type f | grep MOD13A2  | grep A$doy | grep reliability | wc -l)
			if [ $c2 -eq 1 ]
			then
				#NDVI file
				f1=$(find -type f | grep MOD13A2  | grep A$doy | grep NDVI)
				#QA file
				f2=$(find -type f | grep MOD13A2  | grep A$doy | grep reliability)
				#Output filename
				out=ndvi_$doy.tif
				#does it already exist?
				outno=$(find -type f | grep $out | wc -l)

				#Define number of gdalwarp running
				npid=$(echo "$(ps aux | grep ndvi | wc -l) - 1" | bc)
				while [ $npid -ge $ncores ]
				do
					sleep 1
					#Update number of ndvi running
					npid=$(echo "$(ps aux | grep ndvi | wc -l) - 1" | bc)
					#Update number of (virtual) cores (for heterogeneous systems)
					ncores=`grep -c 'processor' /proc/cpuinfo`
				done
				echo -e "\e[01;36m" "ndvi" $f1 $f2 $out "\e[00m"
				#process
				$prog_root/ndvi $f1 $f2 $out &
			fi
		fi
	fi
done


file=$(ls ndvi_* | head -1)
xpix=$(gdalinfo $(echo $file) | grep Pixel | sed 's/Pixel\ Size\ =\ (\(.*\),\(.*\))/\1/g')
ypix=$(gdalinfo $(echo $file) | grep Pixel | sed 's/Pixel\ Size\ =\ (\(.*\),-\(.*\))/\2/g')
echo $xpix $ypix
xmin=$(gdalinfo $(echo $file) | grep "Lower\ Left" | sed 's/Lower\ Left\ \ (\ \ \(.*\)\,\ \ \(.*\),\(.*\)/\1/g')  
xmax=$(gdalinfo $(echo $file) | grep "Upper\ Right" | sed 's/Upper\ Right\ (\ \ \(.*\)\,\ \ \(.*\),\(.*\)/\1/g')
ymin=$(gdalinfo $(echo $file) | grep "Lower\ Left" | sed 's/Lower\ Left\ \ (\ \ \(.*\)\,\ \ \(.*\))\ (\(.*\),\(.*\)/\2/g') 
ymax=$(gdalinfo $(echo $file) | grep "Upper\ Right" | sed 's/Upper\ Right\ (\ \ \(.*\)\,\ \ \(.*\))\ (\(.*\),\(.*\)/\2/g')
echo $xmin $xmax $ymin $ymax
height=$(gdalinfo $(echo $file) | grep 'Size is' | sed 's/Size\ is\ \(.*\),\(.*\)/\2/g')
width=$(gdalinfo $(echo $file) | grep 'Size is' | sed 's/Size\ is\ \(.*\),\(.*\)/\1/g')
#Go to SRTM Data Directory
#mkdir -p ~/DATA/SRTM
cd ~/DATA/SRTM
#cp -f ~/SRTM/srtm_tonlesap.tif ~/DATA/SRTM/srtm.tif
#cp -f ~/Osaka/SRTM/srtm_tonlesap.tif ~/DATA/SRTM/srtm.tif
#merge tiles and reproject
rm -f srtm.tif
#gdalwarp -of GTiff -ot Float32 -t_srs $WGS84 -te $xmin $ymin $xmax $ymax -tr $xpix $ypix ~/Osaka/SRTM/srtm_tonlesap.tif srtm.tif
gdalwarp -of GTiff -ot Float32 -t_srs $WGS84 -te $xmin $ymin $xmax $ymax -ts $width $height ~/DATA/SRTM/srtm_LK.tif ~/DATA/SRTM/srtm.tif

#Go to Data Directory
cd ~/DATA

#ALBEDO PROCESSING
prog_root=$PRoot/prog/prog_ALB
cd $prog_root
make clean
make
cd ~/DATA
dfs=0.25

for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "Alb" $doy
		#count tarball identified
		c1=$(find -type f | grep MCD43B3  | grep A$doy | grep WSA_shortwave.tif | wc -l)
		if [ $c1 -eq 1 ]
		then
			#count tarball identified
			c2=$(find -type f | grep MCD43B3  | grep A$doy | grep BSA_shortwave.tif | wc -l)
			if [ $c2 -eq 1 ]
			then
				#count tarball identified
				c3=$(find -type f | grep MCD43B2  | grep A$doy | grep Albedo_Quality | wc -l)
				if [ $c3 -eq 1 ]
				then
					echo "Found all MCDs"
					#BSA file
					#find tarball
					f1=$(find -type f | grep MCD43B3  | grep A$doy | grep WSA_shortwave.tif)
					#WSA file
					#find tarball
					f2=$(find -type f | grep MCD43B3  | grep A$doy | grep BSA_shortwave.tif)
					#QA file
					#find tarball
					f3=$(find -type f | grep MCD43B2  | grep A$doy | grep Albedo_Quality)
					#Output filename
					out=alb_$doy.tif
					#does it already exist?
					outno=$(find -type f | grep $out | wc -l)

					#Define number of gdalwarp running
					npid=$(echo "$(ps aux | grep alb | wc -l) - 1" | bc)
					while [ $npid -ge $ncores ]
					do
						sleep 1
						#Update number of alb running
						npid=$(echo "$(ps aux | grep alb | wc -l) - 1" | bc)
						#Update number of (virtual) cores (for heterogeneous systems)
						ncores=`grep -c 'processor' /proc/cpuinfo`
					done
					echo -e "\e[01;36m" "alb" $out $dfs $f1 $f2 $f3 "\e[00m"
					#process
					$prog_root/alb $out $dfs $f1 $f2 $f3 &
				fi
			fi
		fi
	fi
done

#LST PROCESSING
prog_root=$PRoot/prog/prog_LST
cd $prog_root
make clean
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "lst" $doy
		#LST Day file
		#count tarball identified
		c1=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_LST | wc -l)
		if [ $c1 -eq 1 ]
		then
			#count tarball identified
			c2=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_QC | wc -l)
			if [ $c2 -eq 1 ]
			then
				#find tarball
				f1=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_LST)
				#QC file
				#find tarball
				f2=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_QC)
				#Output filename
				out=lst_$doy.tif
				#does it already exist?
				outno=$(find -type f | grep $out | wc -l)

				#Define number of gdalwarp running
				npid=$(echo "$(ps aux | grep lst | wc -l) - 1" | bc)
				while [ $npid -ge $ncores ]
				do
					sleep 1
					#Update number of lst running
					npid=$(echo "$(ps aux | grep lst | wc -l) - 1" | bc)
					#Update number of (virtual) cores (for heterogeneous systems)
					ncores=`grep -c 'processor' /proc/cpuinfo`
				done
				echo -e "\e[01;36m" "lst" $f1 $f2 $out "\e[00m"
				#process
				$prog_root/lst $f1 $f2 $out &
			fi
		fi
	fi
done

#T0dem PROCESSING
prog_root=$PRoot/prog/prog_T0Dem
cd $prog_root
make clean
make
cd ~/DATA
#DEM file
f0=~/DATA/SRTM/srtm.tif

for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "t0dem" $doy
		#count tarball identified
		c1=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_LST | wc -l)
		if [ $c1 -eq 1 ]
		then
			#find tarball
			c2=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_QC | wc -l)
			if [ $c2 -eq 1 ]
			then
				#LST Day file
				f1=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_LST)
				#QC file
				f2=$(find -type f | grep MOD11A1  | grep A$doy | grep LST_QC)
				#Output filename
				out=t0dem_$doy.tif
				#does it already exist?
				outno=$(find -type f | grep $out | wc -l)

				#Define number of gdalwarp running
				npid=$(echo "$(ps aux | grep alb | wc -l) - 1" | bc)
				while [ $npid -ge $ncores ]
				do
					sleep 1
					#Update number of alb running
					npid=$(echo "$(ps aux | grep alb | wc -l) - 1" | bc)
					#Update number of (virtual) cores (for heterogeneous systems)
					ncores=`grep -c 'processor' /proc/cpuinfo`
				done
				echo -e "\e[01;36m" "t0dem" $f0 $f1 $f2 $out "\e[00m"
				#process
				$prog_root/t0dem $f0 $f1 $f2 $out &
			fi
		fi
	fi
done

