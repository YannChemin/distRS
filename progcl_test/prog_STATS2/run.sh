#!/bin/bash
in=../ef/
rm tempfile.txt -f
pre=cia_theta_
echo $pre
for (( doy = 2000000 ; doy < 2011000 ; doy++ ))
	do test1=$(find $in -type f | grep $pre$doy | wc -l)
	if [ $test1 -eq 1 ]
	then 
	# 	echo "./stats" $in/$pre$doy".tif -28768"
		./stats $in/$pre$doy.tif 0 >> tempfile.txt
	fi
done
# pre=cia_eta_
# echo $pre
# for (( doy = 2000000 ; doy < 2011000 ; doy++ ))
# 	do test1=$(find $in -type f | grep $pre$doy | wc -l)
# 	if [ $test1 -eq 1 ]
# 	then 
# 	# 	echo "./stats" $in/$pre$doy".tif -28768"
# 		./stats $in/$pre$doy.tif 0 >> tempfile.txt
# 	fi
# done
