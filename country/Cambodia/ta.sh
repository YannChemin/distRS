#!/bin/bash
#Gap filling for eta data
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR
for year in 2000 2005 2010 2013 2014
do
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/eta_sebal_$year*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)eta_sebal_\(.*\)/eta_sebal_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/eta_sebal_*.tif ~/DATA/
done

#Gap filling for LST data
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR
for year in 2000 2005 2010 2013 2014
do
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/lst_$year*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)lst_\(.*\)/lst_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make clean
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/lst_*.tif ~/DATA/
done



#Gap filling for NDVI data
inDIR=~/DATA
outDIR=~/DATA/ta_int
mkdir -p $outDIR
inLISTDIR=""
outLISTDIR=""
inLISTDIR=$(ls $inDIR/ndvi_*.tif)
for file in $inLISTDIR
do
 infile=$(echo $file | sed 's/\(.*\)ndvi_\(.*\)/ndvi_\2/')
 outLISTDIR=$outLISTDIR" "$outDIR/$infile
done
cd ~/dev/distRS/trunk/prog/prog_filling
make clean
make
./filling $inLISTDIR $outLISTDIR
mv ~/DATA/ta_int/ndvi_*.tif ~/DATA/

#Gap filling for ALB data
 inDIR=~/DATA
 outDIR=~/DATA/ta_int
 mkdir -p $outDIR
 inLISTDIR=""
 outLISTDIR=""
 inLISTDIR=$(ls $inDIR/alb_*.tif)
 for file in $inLISTDIR
 do
  infile=$(echo $file | sed 's/\(.*\)alb_\(.*\)/alb_\2/')
  outLISTDIR=$outLISTDIR" "$outDIR/$infile
 done
 cd ~/dev/distRS/trunk/prog/prog_filling
 make clean
 make
 ./filling $inLISTDIR $outLISTDIR
 mv ~/DATA/ta_int/alb_*.tif ~/DATA/


bash ~/Desktop/08_transpiration_fc.sh

echo "Please run in GRASS"
cd ~/DATA

for file in eta_sebal*.tif
do
 r.in.gdal input=$file output=$(echo $file | sed 's/.tif//') --o --q
 r.null map=$(echo $file | sed 's/.tif//') setnull=-28768
done

for file in fc_*.tif
do
 r.in.gdal input=$file output=$(echo $file | sed 's/.tif//') --o --q
done

for file in ta_*.tif
do
 r.in.gdal input=$file output=$(echo $file | sed 's/.tif//') --o --q
 r.null map=$(echo $file | sed 's/.tif//') setnull=-28768
done

#PCA  will require the extension of ulimit for open files in Ubuntu
#sudo vi /etc/security/limits.conf
#There you should enter:
#yann            hard    nofile          65000
#yann            soft    nofile          10000
#before # End of file


i.group group=ta_group input=$(g.mlist type=rast pattern=ta_* sep=,) 
i.segment group=ta_group output=seg_ta threshold=0.9 memory=5000 
iterations=50 --o & 
i.pca input=ta_group output_prefix=pca_ta percent
=90 --o & i.group group=pca_ta_group input=pca_ta.1,pca_ta.2,pca_ta.3
,pca_ta.4,pca_ta.5,pca_ta.6,pca_ta.7,pca_ta.8,pca_ta.9,pca_ta.10,
pca_ta.11,pca_ta.12,pca_ta.13,pca_ta.14,pca_ta.15,pca_ta.16,pca_ta.17
,pca_ta.18,pca_ta.19,pca_ta.20,pca_ta.21,pca_ta.22,pca_ta.23,
pca_ta.24,pca_ta.25,pca_ta.26,pca_ta.27,pca_ta.28,pca_ta.29,pca_ta.30
,pca_ta.31,pca_ta.32,pca_ta.33,pca_ta.34,pca_ta.35,pca_ta.36,
pca_ta.37,pca_ta.38,pca_ta.39,pca_ta.40,pca_ta.41,pca_ta.42,pca_ta.43
,pca_ta.44,pca_ta.45,pca_ta.46,pca_ta.47,pca_ta.48,pca_ta.49,
pca_ta.50,pca_ta.51,pca_ta.52,pca_ta.53,pca_ta.54,pca_ta.55 
#i.group group=pca_ta_group input=$(g.mlist type=rast pattern=pca_ta* sep=,) 
i.segment group=pca_ta_group output=seg_pca_ta threshold=0.9 memory=
5000 iterations=50 --o & 

for year in 2000 2005 2010 2013 2014
do
 #Make Yearly Groups
 i.group group=ta_group_$year input=$(g.mlist type=rast pattern=ta_$year* sep=,) 
 #Make Segmentation for each Year directly
 i.segment group=ta_group_$year output=seg_ta_$year threshold=0.9 memory=5000 iterations=50 --o
 #Make pca of each year
 i.pca input=ta_group_$year output_prefix=pca_ta_$year percent=90 --o 
 #Make group of PCA
 i.group group=pca_ta_group_$year input=$(g.mlist type=rast pattern=pca_ta_$year.[01234][0123456789] sep=,)
 #Make Segmentation for each Year of PCA
 i.segment group=ta_group_$year output=seg_ta_$year threshold=0.9 memory=5000 iterations=50 --o
done

r.mapcalc expression="seg_ta_years=seg_ta_2000+seg_ta_2005+seg_ta_2010+seg_ta_2013+seg_ta_2014"
r.clump -d --overwrite input=seg_ta_years@PERMANENT output=seg_ta_years_clump
r.reclass.area -c --overwrite input=seg_ta_years_clump@PERMANENT output=seg_ta_years_clump_reclass greater=10000
r.to.vect -s --overwrite --quiet input=seg_ta_years_clump_reclass output=seg_ta_years_clump_reclass type=area
#Make cumulative Transpiration
for year in 2000 2005 2010 2013 2014
do
 r.mapcalc expression="ta_sum_$year=$(g.mlist type=rast pattern=ta_$year* sep=+)"
done
r.mapcalc expression="ta_sum=$(g.mlist type=rast pattern=ta_sum_* sep=+)"
#Average cumulate per year
r.mapcalc expression="ta_avg=ta_sum/$(g.mlist type=rast pattern=ta_sum_* | wc -l)"
#Zonal statistics
r.stats.zonal --overwrite --quiet base=seg_ta_years_clump_reclass cover=ta_sum method=average output=seg_ta_years_clump_reclass_ta_sum
r.stats.zonal --overwrite --quiet base=seg_ta_years_clump_reclass cover=ta_avg method=average output=seg_ta_years_clump_reclass_ta_avg
v.rast.stats -c map=seg_ta_years_clump_reclass@PERMANENT raster=ta_sum column_prefix=ta_sum

r.mapcalc expression="tempo=if(seg_ta_years_clump_reclass_ta_avg<1000,null(),seg_ta_years_clump_reclass_ta_avg)"
g.rename tempo,seg_ta_years_clump_reclass_ta_avg --o
r.colors -n -e map=seg_ta_years_clump_reclass_ta_avg color=byr

#Temporal plots (limited to 400 points, not nice)
i.spectral -c -g --overwrite group=ta_group@PERMANENT coordinates=104.118832166,14.7585647484,103.525338069,13.0911289527,103.304897404,12.9328638602 output=/home/yann/dev/grass-promo/grassposter/2014_EGU_WD_Landscape/images/ta_spectral

#R2V 
r.to.vect -s --overwrite --quiet input=seg_ta_years_clump_reclass_ta_avg@PERMANENT output=seg_ta_years_clump_reclass_ta_avg type=area
v.out.ogr --overwrite --quiet input=seg_ta_years_clump_reclass_ta_avg@PERMANENT dsn=/home/yann/Osaka/seg_ta_years_clump_reclass_ta_avg format=ESRI_Shapefile
