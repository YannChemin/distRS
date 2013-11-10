#!/bin/bash
in=/home/yann/Documents/2012_Papers/IJG_water/NEcj
rm hist.csv -f
echo "./hist" $in/NEcj_FREQ.tif" -28768"
./hist $in/NEcj_FREQ.tif -28768 > hist.csv
R CMD BATCH histogram_rice.R