#!/bin/bash

# Show Help
python3 ./readNESZ.py

# Define test params
safedir="/Users/dnd/snap/Cozmin/S1A_IW_SLC__1SDV_20200509T055041_20200509T055108_032484_03C30B_EF9F.SAFE"
outbase="/Users/dnd/snap/noise_"

# Run test
python3 ./readNESZ.py $safedir $outbase
