#!/bin/bash
#-----------------------------------------
#--Modis Processing chain--Serial code----
#-----------------------------------------
#./ndvi inB1 inB2 inQC inB3
#        outNDVI
#-----------------------------------------
#inB1/B2         Modis MOD09GQ B1 B2 250m
#inQC            Modis MOD09GQ QC 250m
#inB3            Modis MOD09GA B3 QC corrected 250m
#outNDVI QA corrected NDVI 250m output [-]
#-----------------------------------------

MODGAproduct=mod09ga
MODGQproduct=mod09gq
DataRootGA=./
DataRootGQ=./
DataOut=/data/GR/MOD_NDVI/
mkdir -p $DataOut
preGA=MOD09GA_
preGQ=MOD09GQ_
program=ndvi
progRoot=$PWD

# get the tif files input/output names
inBQC250m=HDF4_EOS_EOS_GRID__MOD09GQ_A2017270_006_2017272062359_hdf__MODIS_Grid_2D_QC_250m_1.tif
inB1=HDF4_EOS_EOS_GRID__MOD09GQ_A2017270_006_2017272062359_hdf__MODIS_Grid_2D_sur_refl_b01_1.tif
inB2=HDF4_EOS_EOS_GRID__MOD09GQ_A2017270_006_2017272062359_hdf__MODIS_Grid_2D_sur_refl_b02_1.tif
inBQC500m=MOD09GA_2017270_500m_1D_B3_QC_Corrected.tif
out=MOD09_250m_1D_NDVI_QC_250_500_Corrected.tif
echo "./$program $inB1 $inB2 $inBQC250m $inBQC500m $out"
rm -f $out
$progRoot/$program $inB1 $inB2 $inBQC250m $inBQC500m $out &
