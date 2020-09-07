#!/bin/bash

#split creates xaa xab xac xad xae files
for file in $(cat xaa)
do
        bash run1TGZgcp.sh $file
done

