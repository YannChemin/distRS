#!/bin/bash
make clean
make
inB1=HDF4_EOS_EOS_GRID__MOD09GA_A2001001_006_2015140055847_hdf__MODIS_Grid_500m_2D_QC_500m_1.tif
inB2=HDF4_EOS_EOS_GRID__MOD09GA_A2001001_006_2015140055847_hdf__MODIS_Grid_500m_2D_sur_refl_b03_1.tif
out=MOD09GA_2001001.tif
echo "./mod09ga $inB1 $inB2 $out"
rm -f $out
./mod09ga $inB1 $inB2 $out
