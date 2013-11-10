#!/bin/bash
#suf=FarmA
# inX=151.45981
# inY=-27.27545
# suf=FarmB
# inX=150.11465
# inY=-28.55077
# suf=FarmC_OFS_NORTH
# inX=148.1589
# inY=-28.5341
# Big Field North
# suf=FarmC
# inX=148.1738
# inY=-28.5443
#Small Field South
# suf=FarmC_1
# inX=148.1881
# inY=-28.580
# suf=FarmC_OFS_SOUTH
# inX=148.2017
# inY=-28.5791
# suf=FarmD_OFS
# inX=149.96448
# inY=-30.47623
# suf=FarmD
# inX=149.98701
# inY=-30.47189
# suf=FarmE_OFS
# inX=149.77066
# inY=-30.28026
# suf=FarmE
# inX=149.75741
# inY=-30.28141
# suf=FarmF_OFS
# inX=149.0547
# inY=-29.4762
# suf=FarmF
# inX=149.0525
# inY=-29.4965
suf=CIA_Farm5001
inX=145.95757
inY=-34.63494
root=/media/disk2part1/RS/3_Products/eta_sebal

rm -f result_$suf.csv

for file in $root/*tif
do
	echo $file
	./extract $inX $inY $file >> result_$suf.csv
done