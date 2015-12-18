doySTART=2000000
doyEND=2016000
#Go to Data Directory
cd ~/DATA

#Define number of (virtual) cores
ncores=`grep -c 'processor' /proc/cpuinfo`
echo "ncores=" $ncores

PRoot=~/dev/distRS/trunk

#DEM file
f0=~/DATA/SRTM/srtm.tif

#RNET PROCESSING
prog_root=$PRoot/prog/prog_rnet
cd $prog_root
make
cd ~/DATA
Tmax=305.0
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "Rnet" $doy
		if [ $(find -type f | grep alb_  | grep $doy | grep .tif | wc -l) == 1 ]
		then
			#Update Albedo file
			f1=$(find -type f | grep alb_  | grep $doy | grep .tif)
			#count GeoTiff identified
			c1=$(find -type f | grep alb_  | grep $doy | grep .tif | wc -l)
		fi
		if [ $(find -type f | grep sun_zenith  | grep A$doy | grep .tif | wc -l) == 1 ]
		then
			#Update SUNZA file
			f2=$(find -type f | grep sun_zenith  | grep A$doy )
			#count GeoTiff identified
			c2=$(find -type f | grep sun_zenith  | grep A$doy | grep .tif | wc -l)
		fi
		#Emissivity 31 file
		f3=$(find -type f | grep Emis_31  | grep A$doy | grep .tif)
		#count GeoTiff identified
		c3=$(find -type f | grep Emis_31  | grep A$doy | grep .tif | wc -l)

		#Emissivity 32 file
		f4=$(find -type f | grep Emis_32  | grep A$doy | grep .tif)
		#count GeoTiff identified
		c4=$(find -type f | grep Emis_32  | grep A$doy | grep .tif | wc -l)

		#LST file
		f5=$(find -type f | grep lst_  | grep $doy | grep .tif)
		#count GeoTiff identified
		c5=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)

		#Output filename
		out=rnet_$doy.tif
		#does it already exist?
		outno=$(find -type f | grep $out | wc -l)
		echo "1/" $c1 "2/" $c2 "3/" $c3 "4/" $c4 "5/" $c5
#		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $c5 == 1 && $outno == 0 ]]
		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $c5 == 1 ]]
		then
			#Define number of gdalwarp running
			npid=$(echo "$(ps aux | grep r_net | wc -l) - 1" | bc)
			while [ $npid -ge $ncores ]
			do
				sleep 1
				#Update number of lst running
				npid=$(echo "$(ps aux | grep r_net | wc -l) - 1" | bc)
				#Update number of (virtual) cores (for heterogeneous systems)
				ncores=`grep -c 'processor' /proc/cpuinfo`
			done
			echo -e "\e[01;33m" "r_net" $f1 $f2 $f3 $f4 $f5 $f0 $out $d $Tmax "\e[00m"
			d=$(expr $doy % 1000)
			#process
			$prog_root/r_net $f1 $f2 $f3 $f4 $f5 $f0 $out $d $Tmax &
		fi
	fi
done


#G0 PROCESSING
prog_root=$PRoot/prog/prog_g0
cd $prog_root
make
cd ~/DATA
roerink=0
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "g0" $doy
		if [ $(find -type f | grep alb_  | grep $doy | grep .tif | wc -l) == 1 ]
		then
			#Update Albedo file
			f1=$(find -type f | grep alb_  | grep $doy | grep .tif)
			#count GeoTiff identified
			c1=$(find -type f | grep alb_  | grep $doy | grep .tif | wc -l)
		fi
		if [ $(find -type f | grep ndvi_  | grep $doy | grep .tif | wc -l) == 1 ]
		then
			#Update NDVI file
			f2=$(find -type f | grep ndvi_  | grep $doy | grep .tif)
			#count GeoTiff identified
			c2=$(find -type f | grep ndvi_  | grep $doy | grep .tif | wc -l)
		fi
		#LST file
		#find GeoTiff
		f3=$(find -type f | grep lst_  | grep $doy | grep .tif)
		#count GeoTiff identified
		c3=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
		#RNET file
		#find GeoTiff
		f4=$(find -type f | grep rnet_  | grep $doy | grep .tif)
		#count GeoTiff identified
		c4=$(find -type f | grep rnet_  | grep $doy | grep .tif | wc -l)
		#Time file
		#find GeoTiff
		f5=$(find -type f | grep Day_view_time  | grep A$doy | grep .tif)
		#count GeoTiff identified
		c5=$(find -type f | grep Day_view_time  | grep A$doy | grep .tif | wc -l)

		#Output filename
		out=g0_$doy.tif
		#does it already exist?
		outno=$(find -type f | grep $out | wc -l)
		echo "1" $c1 "2" $c2 "3" $c3 "4" $c4 "5" $c5
#		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $c5 == 1 && $outno == 0 ]]
		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $c5 == 1 ]]
		then
			#Define number of g0 running
			npid=$(echo "$(ps aux | grep g_0 | wc -l) - 1" | bc)
			while [ $npid -ge $ncores ]
			do
				sleep 1
				#Update number of ndvi running
				npid=$(echo "$(ps aux | grep g_0 | wc -l) - 1" | bc)
				#Update number of (virtual) cores (for heterogeneous systems)
				ncores=`grep -c 'processor' /proc/cpuinfo`
			done
			echo -e "\e[01;33m" "g_0" $f1 $f2 $f3 $f4 $f5 $out $roerink "\e[00m"
			#process
			$prog_root/g_0 $f1 $f2 $f3 $f4 $f5 $out $roerink &
		fi
	fi
done

#ETo PM PROCESSING
prog_root=$PRoot/prog/prog_ETo_PM
cd $prog_root
make
cd ~/DATA
RH=0.80
u=1.0
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "EToPM" $doy
		#count GeoTiff identified
		c1=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
#		if [[ $c1 == 1 && $c2 == 1 && $outno == 0 ]]
		if [[ $c1 == 1 ]]
		then
			#count GeoTiff identified
			c2=$(find -type f | grep rnet_  | grep $doy | grep .tif | wc -l)
	#		if [[ $c1 == 1 && $c2 == 1 && $outno == 0 ]]
			if [[ $c2 == 1 ]]
			then

				#LST file
				#find GeoTiff
				f1=$(find -type f | grep lst_  | grep $doy | grep .tif)
				#RNET file
				#find GeoTiff
				f2=$(find -type f | grep rnet_  | grep $doy | grep .tif)

				#Output filename
				out=eto_pm_$doy.tif
				#does it already exist?
				outno=$(find -type f | grep $out | wc -l)

				#Define number of g0 running
				npid=$(echo "$(ps aux | grep pm_eto | wc -l) - 1" | bc)
				while [ $npid -ge $ncores ]
				do
					sleep 1
					#Update number of ndvi running
					npid=$(echo "$(ps aux | grep pm_eto | wc -l) - 1" | bc)
					#Update number of (virtual) cores (for heterogeneous systems)
					ncores=`grep -c 'processor' /proc/cpuinfo`
				done
				echo -e "\e[01;33m" "pm_eto" $f1 $f0 $f2 $out $RH $u "\e[00m"
				#process
				$prog_root/pm_eto $f1 $f0 $f2 $out $RH $u &
			fi
		fi
	fi
done

#RNETD PROCESSING
prog_root=$PRoot/prog/prog_rnetd
cd $prog_root
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "Rnetd" $doy
		if [ $(find -type f | grep alb_  | grep $doy | grep .tif | wc -l) == 1 ]
		then
			#Update Albedo file
			#find GeoTiff
			f1=$(find -type f | grep alb_  | grep $doy | grep .tif)
			#count GeoTiff identified
			c1=$(find -type f | grep alb_  | grep $doy | grep .tif | wc -l)
		fi
		#Emissivity 31 file
		#find GeoTiff
		f2=$(find -type f | grep Emis_31  | grep A$doy | grep .tif)
		#count GeoTiff identified
		c2=$(find -type f | grep Emis_31  | grep A$doy | grep .tif | wc -l)
		#Emissivity 32 file
		#find GeoTiff
		f3=$(find -type f | grep Emis_32  | grep A$doy | grep .tif)
		#count GeoTiff identified
		c3=$(find -type f | grep Emis_32  | grep A$doy | grep .tif | wc -l)
		#LST file
		#find GeoTiff
		f4=$(find -type f | grep lst_  | grep $doy | grep .tif)
		#count GeoTiff identified
		c4=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
		#Output filename
		out=rnetd_$doy.tif
		#does it already exist?
		outno=$(find -type f | grep $out | wc -l)
# 		echo "1" $c1 "2" $c2 "3" $c3 "4" $c4
#		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 && $outno == 0 ]]
		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 && $c4 == 1 ]]
		then
			#Define number of g0 running
			npid=$(echo "$(ps aux | grep r_netd | wc -l) - 1" | bc)
			while [ $npid -ge $ncores ]
			do
				sleep 1
				#Update number of r_netd running
				npid=$(echo "$(ps aux | grep r_netd | wc -l) - 1" | bc)
				#Update number of (virtual) cores (for heterogeneous systems)
				ncores=`grep -c 'processor' /proc/cpuinfo`
			done
			d=$(expr $doy % 1000)
			echo -e "\e[01;33m" "r_netd" $f1 $f0 $f2 $f3 $f4 $out $d "\e[00m"
			#process
			$prog_root/r_netd $f1 $f0 $f2 $f3 $f4 $out $d&
		fi
	fi
done

