export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

#Offline Orbit files paths
POE=/home/yann/RSDATA/ESAOrbits/
RES=/home/yann/RSDATA/ESAOrbits/

#Path to scratch
#WDR=/scratch2
WDR=.

#data root
DATAROOT=/home/yann/RSDATA/

#Software and configurations
soft=/home/yann/snap/bin/gpt
xml1=$WDR/TOPSAR_Coherence_Single_Swath.xml
xml2=$WDR/TOPSAR_merge_ML_TC.xml

	#for ascdesc in ascending descending
	#do
		file=list.txt 
		cat $file | while read firstline
		do
			echo "$firstline"
			sleep 2
			#Timestamps
			d1=$(echo $firstline | sed 's/\(.*\)\ \(.*\)/\1/' | sed 's/\(.*\)__1SDV_\(.*\)_\(.*\)_\(.*\)_\(.*\)_\(.*\)/\2/')
			d2=$(echo $firstline | sed 's/\(.*\)\ \(.*\)/\2/' | sed 's/\(.*\)__1SDV_\(.*\)_\(.*\)_\(.*\)_\(.*\)_\(.*\)/\2/')
			echo $d1 $d2
			f1=$DATAROOT/$(echo $firstline | sed 's/\(.*\)\ \(.*\)/\1/')
			f2=$DATAROOT/$(echo $firstline | sed 's/\(.*\)\ \(.*\)/\2/')


			for pol1 in HH HV VH VV
			do
				for pol2 in HH HV VH VV
				do
					#Working data files paths
					PTH=$WDR/S1_$pol1\_$pol2\_$d1\_$d2

					sed -i "s/[VH][VH],[VH][VH]/$pol1,$pol2/g" $xml1  
					# Generate subswath coherence for the 3 subswaths
					OUT1=$PTH\_coh_IW1.dim
					OUT2=$PTH\_coh_IW2.dim
					OUT3=$PTH\_coh_IW3.dim
					echo "$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=1 -Pout=$OUT1 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES"
					$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=1 -Pout=$OUT1 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES 2>/dev/null
					echo "$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=2 -Pout=$OUT2 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES"
					$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=2 -Pout=$OUT2 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES 2>/dev/null
					echo "$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=3 -Pout=$OUT3 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES"
					$soft $xml1 -Pimaster=$f1 -Pislave=$f2 -Psub=3 -Pout=$OUT3 -DOrbitFiles.sentinel1POEOrbitPath=$POE -DOrbitFiles.sentinel1RESOrbitPath=$RES 2>/dev/null
					# Merge subswaths and terrain correct
					echo "$soft $xml2 -Pfile_path=$PTH"
					$soft $xml2 -Pfile_path=$PTH 2>/dev/null
					# Optionally compress to byte scaled data
					gdal_translate -q -ot Byte -co "COMPRESS=DEFLATE" -co "TILED=YES" -scale 0.0 1.0 1 251 -a_nodata 0 $PTH\_coh_ML_TC.tif $PTH\_coh_ML_TC_byte.tif
				done
			done
		done
	#done
