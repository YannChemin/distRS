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

#ETa SSEB PROCESSING
prog_root=$PRoot/prog/prog_ETa_SSEB
cd $prog_root
make
cd ~/DATA
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
	if [ $(expr $doy % 1000) -lt 366 ]
	then
		echo "ETa SSEB" $doy
		#count GeoTiff identified
 		c1=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
 		if [[ $c1 == 1 ]]
 		then
 			#LST file
 			#find GeoTiff
 			f1=$(find -type f | grep lst_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c2=$(find -type f | grep eto_pm_  | grep $doy | grep .tif | wc -l)
 			if [[ $c2 == 1 ]]
 			then
 				#eto_pm file
 				#find GeoTiff
				f2=$(find -type f | grep eto_pm_  | grep $doy | grep .tif)
 				#Output filename
 				out=evapfr_sseb_$doy.tif
 				out1=eta_sseb_$doy.tif
 				out2=theta_sseb_$doy.tif
 				#does it already exist?
 				outno=$(find -type f | grep $out | wc -l)
 				#Define number of gdalwarp running
 				npid=$(echo "$(ps aux | grep sseb_eta | wc -l) - 1" | bc)
 				while [ $npid -ge $ncores ]
 				do
 					sleep 1
 					#Update number of sseb_eta running
 					npid=$(echo "$(ps aux | grep sseb_eta | wc -l) - 1" | bc)
 					#Update number of (virtual) cores (for heterogeneous systems)
 					ncores=`grep -c 'processor' /proc/cpuinfo`
 				done
 				echo -e "\e[01;34m"
 				echo "sseb_eta" $f1 $f2 $out $out1 $out2
 				echo -e "\e[00m"
 				#process
 				$prog_root/sseb_eta $f1 $f2 $out $out1 $out2 &
 			fi
 		fi
	fi
done

#ETA SEBAL PROCESSING
prog_root=$PRoot/prog/prog_ETa_SEBAL
cd $prog_root
make
cd ~/DATA
tsw=0.7
roh_w=1000.0
u2m=1.5
iteration=10
for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
 	if [ $(expr $doy % 1000) -lt 366 ]
 	then
 		echo "ETa SEBAL" $doy
 		if [ $(find -type f | grep ndvi_  | grep $doy | grep .tif | wc -l) == 1 ]
 		then
 			#Update NDVI file
 			#find GeoTiff
 			f1=$(find -type f | grep ndvi_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c1=$(find -type f | grep ndvi_  | grep $doy | grep .tif | wc -l)
 		fi
 		if [ $(find -type f | grep alb_  | grep $doy | grep .tif | wc -l) == 1 ]
 		then
 			#Update Albedo file
 			#find GeoTiff
 			f2=$(find -type f | grep alb_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c2=$(find -type f | grep alb_  | grep $doy | grep .tif | wc -l)
 		fi
 		#count GeoTiff identified
 		c3=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
 		echo "c1=" $c1 "c2=" $c2 "c3=" $c3
 		if [[ $c1 == 1 && $c2 == 1 && $c3 == 1 ]]
 		then
 			#LST file
 			#find GeoTiff
 			f3=$(find -type f | grep lst_  | grep $doy | grep .tif)
 			#RNETD file
 			#find GeoTiff
 			f4=$(find -type f | grep rnetd_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c4=$(find -type f | grep rnetd_  | grep $doy | grep .tif | wc -l)
 			#RNET file
 			#find GeoTiff
 			f5=$(find -type f | grep rnet_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c5=$(find -type f | grep rnet_  | grep $doy | grep .tif | wc -l)
 			#G0 file
 			#find GeoTiff
 			f6=$(find -type f | grep g0_  | grep $doy | grep .tif)
 			#count GeoTiff identified
 			c6=$(find -type f | grep g0_  | grep $doy | grep .tif | wc -l)

 			#Output filename
 			out=evapfr_sebal_$doy.tif
 			out1=eta_sebal_$doy.tif
 			out2=dtair_sebal_$doy.tif
 			out3=theta_sebal_$doy.tif
 			#does it already exist?
 			outno=$(find -type f | grep $out | wc -l)
 			echo "c4=" $c4 "c5=" $c5 "c6=" $c6
 # 			if [[ $c4 == 1 && $c5 == 1 && $c6 == 1 && $outno == 0 ]]
 			if [[ $c4 == 1 && $c5 == 1 && $c6 == 1 ]]
 			then
 				#Define number of g0 running
 				npid=$(echo "$(ps aux | grep sebal_eta | wc -l) - 1" | bc)
 				while [ $npid -ge $ncores ]
 				do
 					sleep 1
 					#Update number of ndvi running
 					npid=$(echo "$(ps aux | grep sebal_eta | wc -l) - 1" | bc)
 					#Update number of (virtual) cores (for heterogeneous systems)
 					ncores=`grep -c 'processor' /proc/cpuinfo`
 				done
 				d=$(expr $doy % 1000)
 				echo -e "\e[01;34m"
 				echo "sebal_eta" $f1 $f2 $f3 $f0 $f4 $f5 $f6 $out $out1 $out2 $out3 $tsw $d $roh_w $u2m $iteration
 				echo -e "\e[00m"
 				#process
 				$prog_root/sebal_eta $f1 $f2 $f3 $f0 $f4 $f5 $f6 $out $out1 $out2 $out3 $tsw $d $roh_w $u2m $iteration &
 			fi
 		fi
 	fi
done

# #ET POT d PROCESSING
prog_root=$PRoot/prog/prog_etpotd
cd $prog_root
make
cd ~/DATA
roh_w=1003.0

for (( doy = $doySTART ; doy <= $doyEND ; doy ++ ))
do
 	if [ $(expr $doy % 1000) -lt 366 ]
 	then
 		echo "ET POTd" $doy
 		#count GeoTiff identified
 		c2=$(find -type f | grep lst_  | grep $doy | grep .tif | wc -l)
 		if [[ $c2 == 1 ]]
 		then
 			if [ $(find -type f | grep rnetd_  | grep $doy | grep .tif | wc -l) == 1 ]
 			then
 				#Update NDVI file
 				#find GeoTiff
 				f1=$(find -type f | grep rnetd_  | grep $doy | grep .tif)
 				#count GeoTiff identified
 				c1=$(find -type f | grep rnetd_  | grep $doy | grep .tif | wc -l)
 			fi
 			#LST file
 			#find GeoTiff
 			f2=$(find -type f | grep lst_  | grep $doy | grep .tif)

 			#Output filename
 			out=et_potd_$doy.tif
 			#does it already exist?
 			outno=$(find -type f | grep $out | wc -l)
 	#		if [[ $c1 == 1 && $c2 == 1 && $outno == 0 ]]
 			if [[ $c1 == 1 ]]
 			then
 				#Define number of g0 running
 				npid=$(echo "$(ps aux | grep et_pot_d | wc -l) - 1" | bc)
 				while [ $npid -ge $ncores ]
 				do
 					sleep 1
 					#Update number of ndvi running
 					npid=$(echo "$(ps aux | grep et_pot_d | wc -l) - 1" | bc)
 					#Update number of (virtual) cores (for heterogeneous systems)
 					ncores=`grep -c 'processor' /proc/cpuinfo`
 				done
 				d=$(expr $doy % 1000)
 				echo -e "\e[01;34m"
 				echo "et_pot_d" $f1 $f2 $out $roh_w
 				echo -e "\e[00m"
 				#process
 				$prog_root/et_pot_d $f1 $f2 $out $roh_w &
 			fi
 		fi
 	fi
done

