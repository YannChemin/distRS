#!/bin/bash

progroot=~/dev/distRS/trunk/prog/prog_avg/
cd $progroot
make clean
make

outroot=~/TaGapTonleSap/seasonaly/

dataroot=/home/yann/TaGapTonleSap/daily/
cd $dataroot

#-----------------------
#Process yearly averages
#-----------------------

#for year in 2000 2001 2002 2003 2004 2005 2006 2007 2008 2009 2010 2011 2012 2013 2014
#do files=$(ls ta_gap_$year*.tif)
#$progroot/avg $outroot/avg_ta_gap_$year.tif $files
#done

#cd $outroot
#mv *.tif /home/yann/Dropbox/water\ dynamics/TaGap_TonleSap/yearly/

#------------------------
#Process monthly averages
#------------------------

#for month in 1 2 3 4 5 6 7 8 9 10 11 12
#do list="" ; echo $month
 #if [ $month -eq 1 ]
 #then
  #doystart=1
  #doystop=31
 #elif [ $month -eq 2 ]
 #then
  #doystart=32
  #doystop=59
 #elif [ $month -eq 3 ]
 #then
  #doystart=60
  #doystop=90
 #elif [ $month -eq 4 ]
 #then
  #doystart=91
  #doystop=120
 #elif [ $month -eq 5 ]
 #then
  #doystart=121
  #doystop=151
 #elif [ $month -eq 6 ]
 #then
  #doystart=152
  #doystop=181
 #elif [ $month -eq 7 ]
 #then
  #doystart=182
  #doystop=212
 #elif [ $month -eq 8 ]
 #then
  #doystart=213
  #doystop=243
 #elif [ $month -eq 9 ]
 #then
  #doystart=244
  #doystop=273
 #elif [ $month -eq 10 ]
 #then
  #doystart=274
  #doystop=304
 #elif [ $month -eq 11 ]
 #then
  #doystart=305
  #doystop=334
 #elif [ $month -eq 12 ]
 #then
  #doystart=335
  #doystop=365
 #fi
 
 #list=""
 #for (( doy = doystart ; doy <= doystop ; doy++ ))
 #do echo $doy
  #if [ $doy -lt 10 ];
  #then doyear=00$doy ; echo $doyear
  #elif [ $doy -lt 100 ] && [ $doy -ge 10 ];
  #then doyear=0$doy ; echo $doyear
  #else [ $doy -ge 100 ] && [ $doy -le 366 ];
  #then doyear=$doy ; echo $doyear
  #fi
  #for file in $(find . -type f | grep ta_gap_20 |  grep $doyear.tif)
  #do list=$list" "$file
  #done
 #done
 #echo "LIST IS DONE" 
 #echo $list
 #if [ $month -lt 10 ]
 #then $progroot/avg ta_gap_month_0$month $list
 #else $progroot/avg ta_gap_month_$month $list
 #fi
#done

#------------------------
#Process seasons averages
#------------------------

#wet season
doystart=121
doystop=334

#dry_doystart=335
#dry_doystop=120

list=""

for (( doy = doystart ; doy <= doystop ; doy++ ))
do echo $doy
  if [ $doy -lt 10 ];
  then doyear=00$doy ; echo $doyear
  elif [ $doy -lt 100 ] && [ $doy -ge 10 ];
  then doyear=0$doy ; echo $doyear
  elif [ $doy -ge 100 ] && [ $doy -le 366 ];
  then doyear=$doy ; echo $doyear
  fi
  for file in $(find . -type f | grep ta_gap_20 |  grep $doyear.tif)
  do list=$list" "$file
  done
done
echo "LIST IS DONE" 
echo $list
$progroot/avg $outroot/ta_gap_season_wet.tif $list

cd $outroot
mv *.tif /home/yann/Dropbox/water\ dynamics/TaGap_TonleSap/seasonaly/
