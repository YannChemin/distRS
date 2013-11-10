#!/bin/bash
DataRoot=~/RSDATA/ETb/
root=$DataRoot/3_Products/

pre=MCD12Q1_

in_lulc1=$DataRoot/2_PreProcessed/LULC1/MDB/
in_lulc1_qa=$DataRoot/2_PreProcessed/LULC1_QA/MDB/
out_lulc1=$root/LULC1/
mkdir $out_lulc1 -p
rm $out_lulc1/*.tif -f

in_lulc2=$DataRoot/2_PreProcessed/LULC2/MDB/
in_lulc2_qa=$DataRoot/2_PreProcessed/LULC2_QA/MDB/
out_lulc2=$root/LULC2/
mkdir $out_lulc2 -p
rm $out_lulc2/*.tif -f

in_lulc3=$DataRoot/2_PreProcessed/LULC3/MDB/
in_lulc3_qa=$DataRoot/2_PreProcessed/LULC3_QA/MDB/
out_lulc3=$root/LULC3/
mkdir $out_lulc3 -p
rm $out_lulc3/*.tif -f

in_lulc4=$DataRoot/2_PreProcessed/LULC4/MDB/
in_lulc4_qa=$DataRoot/2_PreProcessed/LULC4_QA/MDB/
out_lulc4=$root/LULC4/
mkdir $out_lulc4 -p
rm $out_lulc4/*.tif -f

in_lulc5=$DataRoot/2_PreProcessed/LULC5/MDB/
in_lulc5_qa=$DataRoot/2_PreProcessed/LULC5_QA/MDB/
out_lulc5=$root/LULC5/
mkdir $out_lulc5 -p
rm $out_lulc5/*.tif -f

for (( doy == 2000 ; doy < 2011 ; doy ++ )) 
do
	test11=$(find $in_lulc1 -type f | grep $pre$doy | wc -l)
	test12=$(find $in_lulc1_qa -type f | grep $pre$doy | wc -l)
	test13=$(find $out_lulc1 -type f | grep $out_lulc1$pre$doy\_500m_366D_LULC1.tif | wc -l)
	if [ $test11 -eq 1 -a $test12 -eq 1 -a $test13 -eq 0 ]
	then in1B2=$in_lulc1$pre$doy\_500m_366D_LULC1.tif
		in1B3=$in_lulc1_qa$pre$doy\_500m_366D_LULC1_QA.tif
		out1LULC=$out_lulc1$pre$doy\_500m_366D_LULC1.tif
		echo "./lulc $in1B2 $in1B3 $out1LULC"
		./lulc $in1B2 $in1B3 $out1LULC &
	fi
	test21=$(find $in_lulc2 -type f | grep $pre$doy | wc -l)
	test22=$(find $in_lulc2_qa -type f | grep $pre$doy | wc -l)
	test23=$(find $out_lulc2 -type f | grep $out_lulc2$pre$doy\_500m_366D_LULC2.tif | wc -l)
	if [ $test21 -eq 1 -a $test22 -eq 1 -a $test23 -eq 0 ]
	then in2B2=$in_lulc2$pre$doy\_500m_366D_LULC2.tif
		in2B3=$in_lulc2_qa$pre$doy\_500m_366D_LULC2_QA.tif
		out2LULC=$out_lulc2$pre$doy\_500m_366D_LULC2.tif
		echo "./lulc $in2B2 $in2B3 $out2LULC"
		./lulc $in2B2 $in2B3 $out2LULC &
	fi
	test31=$(find $in_lulc3 -type f | grep $pre$doy | wc -l)
	test32=$(find $in_lulc3_qa -type f | grep $pre$doy | wc -l)
	test33=$(find $out_lulc3 -type f | grep $out_lulc3$pre$doy\_500m_366D_LULC3.tif | wc -l)
	if [ $test31 -eq 1 -a $test32 -eq 1 -a $test33 -eq 0 ]
	then in3B2=$in_lulc3$pre$doy\_500m_366D_LULC3.tif
		in3B3=$in_lulc3_qa$pre$doy\_500m_366D_LULC3_QA.tif
		out3LULC=$out_lulc3$pre$doy\_500m_366D_LULC3.tif
		echo "./lulc $in3B2 $in3B3 $out3LULC"
		./lulc $in3B2 $in3B3 $out3LULC
	fi
	test41=$(find $in_lulc4 -type f | grep $pre$doy | wc -l)
	test42=$(find $in_lulc4_qa -type f | grep $pre$doy | wc -l)
	test43=$(find $out_lulc4 -type f | grep $out_lulc4$pre$doy\_500m_366D_LULC4.tif | wc -l)
	if [ $test41 -eq 1 -a $test42 -eq 1 -a $test43 -eq 0 ]
	then in4B2=$in_lulc4$pre$doy\_500m_366D_LULC4.tif
		in4B3=$in_lulc4_qa$pre$doy\_500m_366D_LULC4_QA.tif
		out4LULC=$out_lulc4$pre$doy\_500m_366D_LULC4.tif
		echo "./lulc $in4B2 $in4B3 $out4LULC"
		./lulc $in4B2 $in4B3 $out4LULC &
	fi
	test51=$(find $in_lulc5 -type f | grep $pre$doy | wc -l)
	test52=$(find $in_lulc5_qa -type f | grep $pre$doy | wc -l)
	test53=$(find $out_lulc5 -type f | grep $out_lulc5$pre$doy\_500m_366D_LULC5.tif | wc -l)
	if [ $test51 -eq 1 -a $test52 -eq 1 -a $test53 -eq 0 ]
	then in5B2=$in_lulc5$pre$doy\_500m_366D_LULC5.tif
		in5B3=$in_lulc5_qa$pre$doy\_500m_366D_LULC5_QA.tif
		out5LULC=$out_lulc5$pre$doy\_500m_366D_LULC5.tif
		echo "./lulc $in5B2 $in5B3 $out5LULC"
		./lulc $in5B2 $in5B3 $out5LULC &
	fi
done
