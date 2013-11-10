#!/bin/bash

# -----------------------------------------
# --Modis Processing chain--Serial code----
# -----------------------------------------
# ./eta inNDVI inAlbedo
#         inB8 inB14 inB15 inB16 inB17
#         outEVAPFR outETA outDTAIR
#         tsw doy roh_w u@2m
# -----------------------------------------
# inB1            Modis NDVI 1Km
# inB2            Modis Albedo 1Km
# inB8            Modis LST day 1Km
# inB14           Digital Elevation Model 1Km [m]
# inB15           Modis Diurnal Averaged Net Radiation RNETD 1Km [W/m2]
# inB16           Modis Satellite overpass net radiation RNET 1Km [W/m2]
# inB17           Modis Satellite overpass soil heat flux G0 1Km [W/m2]
# outEVAPFR       Evaporative Fraction output [-]
# outETA          Actual ET output [mm/d]
# outDTair        DTair output [K]
# outTHETA        SOIL MOISTURE output [cm3/cm3]
# tsw             Atmospheric single-way transmissivity [-]
# doy             Day of Year [1-366]
# roh_w           Bulk density of water [kg/m3]
# u@2m            Wind Speed at 2 meters height [m/s]
# iteration       Number of SEBAL h0 iterations (3-10)

root=../
outDIR2=../theta_sebal/
outDIR1=../dtair_sebal/
outDIR0=../evapfr_sebal/
outDIR=../eta_sebal/
mkdir $outDIR
mkdir $outDIR0
mkdir $outDIR1
mkdir $outDIR2
rm $outDIR/*.tif -f
rm $outDIR0/*.tif -f
rm $outDIR1/*.tif -f
rm $outDIR2/*.tif -f
touch log.dat
mv logold3.dat logold4.dat -f
mv logold2.dat logold3.dat -f
mv logold1.dat logold2.dat -f
mv logold.dat logold1.dat -f
mv log.dat logold.dat -f

tsw=0.7
roh_w=1000.0
u2m=2.5
iteration=15
# gnuplot gnuplot.sh&
preNDVI=MOD13A2_
preALB=MCD43B3_
preLST=MOD11A1_
inB14=../DEM/srtm.tif
echo $inB14
for (( doy = 2000000 ; doy <= 2011000 ; doy++ ))
  do test4=$(find $root/NDVI/ -type f | grep $preNDVI$doy | wc -l)
  test5=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
#   echo "*" $doy "ndvi="$test4 "albedo="$test5
  if [ $test4 -eq 1 -a $test5 -eq 1 ]
   then inNDVI=../NDVI/$preNDVI$doy\_1Km_16D_NDVI.tif
   inAlbedo=../albedo/$preALB$doy\_1Km_16D_ALB.tif
   for (( ddoy = doy ; ddoy <= doy+15 ; ddoy++ ))
    do test0=$(find $root/rnetd/ -type f | grep rnetd_$ddoy | wc -l)
    test1=$(find $root/LST/ -type f | grep $preLST$ddoy | wc -l)
    test2=$(find $root/rnet/ -type f | grep rnet_$ddoy | wc -l)
    test3=$(find $root/g0/ -type f | grep g0_$ddoy | wc -l)
    test6=$(find $outDIR -type f | grep eta_sebal_$ddoy.tif | wc -l)
    test7=$(find $outDIR0 -type f | grep evapfr_sebal_$ddoy.tif | wc -l)
    test8=$(find $outDIR1 -type f | grep dtair_sebal_$ddoy.tif | wc -l)
    test9=$(find $outDIR2 -type f | grep theta_sebal_$ddoy.tif | wc -l)
#     echo "*" $doy $ddoy "rnetd="$test0 "lst="$test1 "rnet="$test2 "g0="$test3
    if [ $test0 -eq 1 -a $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 1 ]
     then echo "Found Daily@" $ddoy
     inB8=../LST/$preLST$ddoy\_1Km_1D_LST.tif
     inB15=../rnetd/rnetd_$ddoy.tif
     inB16=../rnet/rnet_$ddoy.tif
     inB17=../g0/g0_$ddoy.tif
     outEVAPFR=$outDIR0/evapfr_sebal_$ddoy.tif
     outETA=$outDIR/eta_sebal_$ddoy.tif
     outDTAIR=$outDIR1/dtair_sebal_$ddoy.tif
     outTHETA=$outDIR2/theta_sebal_$ddoy.tif
     d=$(expr $ddoy % 1000)
#      echo $d
#      echo "./sebal_eta $inNDVI $inAlbedo $inB8 $inB14 $inB15 $inB16 $inB17 $outEVAPFR $outETA $outDTAIR $outTHETA $tsw $d $roh_w $u2m $iteration"
     ./sebal_eta $inNDVI $inAlbedo $inB8 $inB14 $inB15 $inB16 $inB17 $outEVAPFR $outETA $outDTAIR $outTHETA $tsw $d $roh_w $u2m $iteration
    fi
   done
  fi
done

cat log.dat
# gnuplot gnuplot.sh&

