#!/bin/bash

root=/home/user/Dir/
files=$(ls $root/*satelliteimages*)
./freq outFREQ $files  
