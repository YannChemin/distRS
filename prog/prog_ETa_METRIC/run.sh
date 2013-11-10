#!/bin/bash

#------------------------------------------------
#-----------------------------------------
#--Modis Processing chain--OpenMP code----
#-----------------------------------------
#./metric_eta inNdvi inLai inLst inAlb
#        outMetric_eta
#-----------------------------------------
#        doy Tsw Ta u z h eto_alf kc
#        [-mproj/-mcolrow/-mauto]

#Below are wet/dry pixels modes
#---------------------------------------------
#-mproj projXwet ProjYwet projXdry projYdry       Manual wet/dry pixel mode (projected)
#-mcolrow Xwet Ywet Xdry Ydry     Manual wet/dry pixel mode (NOT projected)
#-mauto   Automatic seek wet/dry pixel mode (Careful!)
#-----------------------------------------------

DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
outDIR=$root/eta_metric/
outDIR1=$root/evapfr_metric/
outDIR2=$root/dtair_metric/
outDIR3=$root/theta_metric/
mkdir $outDIR
mkdir $outDIR1
mkdir $outDIR2
mkdir $outDIR3
rm $outDIR/*.tif -f
rm $outDIR1/*.tif -f
rm $outDIR2/*.tif -f
rm $outDIR3/*.tif -f

preNDVI=MOD13A2_
preLAI=MCD15A2_
preLST=MOD11A1_
preALB=MCD43B3_

inDEM=$DataRoot/1_HDF/DEM/srtm.tif

# Tsw=0.7 
Ta=293.0 
RH=0.30
u=3.0
z=2.0 
h=1.0 
eto_alf=0.4
kc=0.7

for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
  do test1=$(find $root/NDVI/ -type f | grep $preNDVI$doy | wc -l)
  test2=$(find $root/albedo/ -type f | grep $preALB$doy | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 ]
   then inNDVI=$root/NDVI/$preNDVI$doy\_1Km_16D_NDVI.tif
   inALB=$root/albedo/$preALB$doy\_1Km_16D_ALB.tif
#    echo "found" $doy $inNDVI $inALB
   for (( ddoy = doy ; ddoy <= doy+8 ; ddoy++ ))
   do test3=$(find $root/LAI/ -type f | grep $preLAI$ddoy | wc -l)
    if [ $test3 -eq 1 ]
     then 
#      echo "Found LAI Daily@" $ddoy
     inLAI=$root/LAI/$preLAI$ddoy\_1Km_8D_LAI.tif
     for (( dddoy = ddoy ; dddoy <= ddoy+7 ; dddoy++ ))
     do test4=$(find $root/LST/ -type f | grep $preLST$dddoy | wc -l)
      test5=$(find $outDIR -type f | grep eta_metric_$dddoy.tif | wc -l)
      test6=$(find $outDIR1 -type f | grep evapfr_metric_$dddoy.tif | wc -l)
      test7=$(find $outDIR2 -type f | grep dtair_metric_$dddoy.tif | wc -l)
      test7=$(find $outDIR3 -type f | grep theta_metric_$dddoy.tif | wc -l)
      if [ $test4 -eq 1 ]
       then 
#        echo "Found LST Daily@" $dddoy
       inLST=$root/LST/$preLST$dddoy\_1Km_1D_LST.tif
       outMETRIC_ETA=$outDIR/eta_metric_$dddoy.tif
       outMETRIC_EVAPFR=$outDIR1/evapfr_metric_$dddoy.tif
       outMETRIC_DTAIR=$outDIR2/dtair_metric_$dddoy.tif
       outMETRIC_THETA=$outDIR3/theta_metric_$dddoy.tif
       dd=$(expr $ddoy % 1000)
       year=$(expr $ddoy / 1000 )  
#        Ta=$(./fetch_silo_met_data T.Max $year $doy)
#        RH=$(./fetch_silo_met_data RHmaxT $doy)
#        u=$(./fetch_met_data wspeed $doy) //NO WIND SPEED for SILO DATA
#        echo "./metric_eta $inNDVI $inLAI $inLST $inALB $inDEM $outMETRIC_ETA $outMETRIC_EVAPFR $outMETRIC_DTAIR $dd $Ta $RH $u $z $h $eto_alf $kc -mauto"
       ./metric_eta $inNDVI $inLAI $inLST $inALB $inDEM $outMETRIC_ETA $outMETRIC_EVAPFR $outMETRIC_DTAIR $outMETRIC_THETA $dd $Ta $RH $u $z $h $eto_alf $kc -mauto
       echo $dd
      fi
     done
    fi
   done
  fi
done
