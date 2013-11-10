#!/bin/bash

#------------------------------------------------
#./r_netd $inAlb $inDem  $outRNETD $doy
#-----------------------------------------------
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/rnetd/
mkdir $outDIR
rm $outDIR/*.tif -f

preALB=MCD43B3_
preLST=MOD11A1_

inDem=$DataRoot/1_HDF/DEM/srtm.tif

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
 do test1=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
  test2=$(find $outDIR -type f | grep $outDIR/rnetd_$doy.tif | wc -l)
#   echo $doy $test1 $test2
  if [ $test1 -eq 1 -a $test2 -eq 0 ]
  then inAlb=$root/albedo/$preALB$doy\_1Km_16D_ALB.tif
   echo "found" $doy $inAlb
   for (( ddoy = doy ; ddoy <= doy+15 ; ddoy++ ))
   do test3=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e31 | wc -l)
    test4=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e32 | wc -l)
    test5=$(find $root/LST/ -type f | grep $preLST$ddoy | wc -l)
    test6=$(find $outDIR -type f | grep rnetd_$ddoy.tif | wc -l)
    if [ $test3 -eq 1 -a $test4 -eq 1 -a $test5 -eq 1 -a $test6 -eq 0 ]
     then echo "Found Daily@" $ddoy
     inEmis31=$root/emissivity/$preLST$ddoy\_1Km_1D_e31.tif
     inEmis32=$root/emissivity/$preLST$ddoy\_1Km_1D_e32.tif
     inLst=$root/LST/$preLST$ddoy\_1Km_1D_LST.tif
     outRNETD=$outDIR/rnetd_$ddoy.tif
     d=$(expr $ddoy % 1000)
   echo "./r_netd $inAlb $inDem $inEmis31 $inEmis32 $inLst $outRNETD $d"
     ./r_netd $inAlb $inDem $inEmis31 $inEmis32 $inLst $outRNETD $d
    fi
   done
 fi
done


