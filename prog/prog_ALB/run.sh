#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/
in_alb=$DataRoot/2_PreProcessed/albedo/MDB/
in_alb_qa=$DataRoot/2_PreProcessed/albedo_QA/MDB/
out_alb=$root/albedo/
mkdir $out_alb

pre=MCD43B3_
suf1=_1Km_16D_ALB_
bsa=BSA
wsa=WSA
suf2=.tif

preQA=MCD43B2_
sufQA=_1Km_16D_ALB_QA.tif

out_suf=_1Km_16D_ALB.tif

dfs=0.25

rm $out_alb/*.tif -f
for (( doy = 2010000 ; doy <= 2011000 ; doy++ ))
do test1=$(find $in_alb -type f | grep $pre$doy | grep $wsa | wc -l)
  test2=$(find $in_alb -type f | grep $pre$doy | grep $bsa | wc -l)
  test3=$(find $in_alb_qa -type f | grep $preQA$doy | wc -l)
  test4=$(find $out_alb -type f | grep $pre$doy$out_suf | wc -l)
  if [ $test1 -eq 1 -a $test2 -eq 1 -a $test3 -eq 1 ]
   then echo "./alb" $out_alb$pre$doy$out_suf $dfs $in_alb$pre$doy$suf1$wsa$suf2 $in_alb$pre$doy$suf1$bsa$suf2 $in_alb_qa$preQA$doy$sufQA
   ./alb $out_alb$pre$doy$out_suf $dfs $in_alb$pre$doy$suf1$wsa$suf2 $in_alb$pre$doy$suf1$bsa$suf2 $in_alb_qa$preQA$doy$sufQA
  fi
done

