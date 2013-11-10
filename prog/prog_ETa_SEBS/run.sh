#!/bin/bash

# -----------------------------------------
# --Modis Processing chain--OpenMP code----
# -----------------------------------------
# ./sebs_eta inLst inDem inE031 inE032 inNdvi inAlbedo inSunza
#         outSebs_evapfr outSebs_eta
#         z_pbl t_s p_s u_s hr_s alt_ms doy
# -----------------------------------------
#OVEWRITE MODE Enabled=>1 Disabled=>0
OVERWRITE=1

#This is temporary, should find a way to feed this for each doy/location
z_pbl=200.0 
t_s=300.0 
p_s=101300.0 
u_s=2.0 
hr_s=0.5
alt_ms=120.0

root=../
outDIR0=../evapfr_sebs/
outDIR=../eta_sebs/
mkdir $outDIR0
mkdir $outDIR
rm $outDIR0/*.tif -f
rm $outDIR/*.tif -f
preALB=MCD43B3_
preNDVI=MOD13A2_
preSZA=MOD13A2_
preLST=MOD11A1_
inDem=../DEM/srtm.tif

for (( doy = 2000000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
  test2=$(find $root/sunza/ -type f | grep $preSZA$doy | wc -l)
  test3=$(find $root/NDVI/ -type f | grep $preNDVI$doy | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 1 ]
   then inAlb=$root/albedo/$preALB$doy\_1Km_16D_ALB.tif
   inSunza=$root/sunza/$preSZA$doy\_1Km_16D_SUNZA.tif
   inNDVI=$root/NDVI/$preNDVI$doy\_1Km_16D_NDVI.tif
   echo "found" $doy "ndvi="$inNDVI $inAlb $inSunza
   for (( ddoy = doy ; ddoy <= doy+15 ; ddoy++ ))
   do test4=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e31 | wc -l)
    test5=$(find $root/emissivity/ -type f | grep $preLST$ddoy | grep e32 | wc -l)
    test6=$(find $root/LST/ -type f | grep $preLST$ddoy | wc -l)
    if [ $test4 -eq 1 -a $test5 -eq 1 -a $test6 -eq 1 ]
     then echo "Found Daily@" $ddoy
     inEmis31=$root/emissivity/$preLST$ddoy\_1Km_1D_e31.tif
     inEmis32=$root/emissivity/$preLST$ddoy\_1Km_1D_e32.tif
     inLST=$root/LST/$preLST$ddoy\_1Km_1D_LST.tif
     outSEBS_EVAPFR=$outDIR0/evapfr_sebs_$ddoy.tif
     outSEBS_ETA=$outDIR/eta_sebs_$ddoy.tif
     d=$(expr $ddoy % 1000)
     #Test existance of output files
     test7=$(find $outDIR0 -type f | grep evapfr_sebs_$ddoy.tif | wc -l)
     test8=$(find $outDIR -type f | grep eta_sebs_$ddoy.tif | wc -l)
     #Output files exist and OVERWRITE is TRUE
     if [ $OVERWRITE -eq 1 -a $test7 -eq $OVERWRITE -a $test8 -eq $OVERWRITE ]
      then echo "./sebs_eta $inLST $inDem $inEmis31 $inEmis32 $inNDVI $inAlb $inSunza $outSEBS_EVAPFR $outSEBS_ETA $z_pbl $t_s $p_s $u_s $hr_s $alt_ms $d"
      ./sebs_eta $inLST $inDem $inEmis31 $inEmis32 $inNDVI $inAlb $inSunza $outSEBS_EVAPFR $outSEBS_ETA $z_pbl $t_s $p_s $u_s $hr_s $alt_ms $d
     fi
     #Output Files do not exist yet
     if [ $test7 -eq 0 -a $test8 -eq 0 ]
      then echo "./sebs_eta $inLST $inDem $inEmis31 $inEmis32 $inNDVI $inAlb $inSunza $outSEBS_EVAPFR $outSEBS_ETA $z_pbl $t_s $p_s $u_s $hr_s $alt_ms $d"
      ./sebs_eta $inLST $inDem $inEmis31 $inEmis32 $inNDVI $inAlb $inSunza $outSEBS_EVAPFR $outSEBS_ETA $z_pbl $t_s $p_s $u_s $hr_s $alt_ms $d
     fi
    fi
   done
  fi
 done
done
