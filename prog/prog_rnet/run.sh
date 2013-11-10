#!/bin/bash

#------------------------------------------------
#./r_net $inAlb $inSunza $inEmis31 $inEmis32 $inLst $inDem
#		$outRNET $doy $Tmax
#-----------------------------------------------
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/rnet/
mkdir $outDIR
rm $outDIR/*.tif -f
preALB=MCD43B3_
preSZA=MOD13A2_
preLST=MOD11A1_
Tmax=300.0
inDem=$DataRoot/1_HDF/DEM/srtm.tif

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
  do test1=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
  test2=$(find $root/sunza/ -type f | grep $preSZA$doy | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 ]
   then inAlb=$root/albedo/$preALB$doy\_1Km_16D_ALB.tif
   inSunza=$root/sunza/$preSZA$doy\_1Km_16D_SUNZA.tif
   echo "found" $doy $inAlb $inSunza
   for (( ddoy = doy ; ddoy <= doy+15 ; ddoy++ ))
   do test3=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e31 | wc -l)
    test4=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e32 | wc -l)
    test5=$(find $root/LST/ -type f | grep $preLST$ddoy | wc -l)
    test6=$(find $outDIR -type f | grep rnet_$ddoy.tif | wc -l)
    if [ $test3 -eq 1 -a $test4 -eq 1 -a $test5 -eq 1 -a $test6 -eq 0 ]
     then echo "Found Daily@" $ddoy
     inEmis31=$root/emissivity/$preLST$ddoy\_1Km_1D_e31.tif
     inEmis32=$root/emissivity/$preLST$ddoy\_1Km_1D_e32.tif
     inLst=$root/LST/$preLST$ddoy\_1Km_1D_LST.tif
     outRNET=$outDIR/rnet_$ddoy.tif
     d=$(expr $ddoy % 1000)
#      echo "./r_net $inAlb $inSunza $inEmis31 $inEmis32 $inLst $inDem $outRNET $d $Tmax"
     ./r_net $inAlb $inSunza $inEmis31 $inEmis32 $inLst $inDem $outRNET $d $Tmax
    fi
   done
  fi
done


