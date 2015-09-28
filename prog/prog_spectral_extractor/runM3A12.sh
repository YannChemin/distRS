#!/bin/bash
root=/home/yann/RSDATA/M3/A12/vsmall/
file=$root/A12_vsmall

suf=HeadCrater
rm -f result_$suf.csv
inX=-23.42740
inY=-3.01427
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=IntrepidSurveyor
rm -f result_$suf.csv
inX=-23.42235
inY=-3.01439
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=BlockSnowmanCrater
rm -f result_$suf.csv
inX=-23.41734
inY=-3.01427
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=BelowBlockSnowmanCrater
rm -f result_$suf.csv
inX=-23.41734
inY=-3.01934
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=HaloCrater
rm -f result_$suf.csv
inX=-23.42254
inY=-3.01938
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=BenchCrater
rm -f result_$suf.csv
inX=-23.42767
inY=-3.01955
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done
suf=SharpApolloCrater
rm -f result_$suf.csv
inX=-23.43274
inY=-3.01942
for (( bn = 1 ; bn < 84 ; bn ++ ))
do
	./extract $inX $inY $file $bn >> result_$suf.csv
done

