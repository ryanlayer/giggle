#!/bin/bash
set -o nounset

if [ "$#" -ne "3" ]
then
    echo "useage: $0 <dir> <bed> <genome>"
    exit
fi

DIR=$1
export BED_FILE=$2
export GENOME=$3


#ls $DIR/*gz | xargs -I {} bash -c "bedtools intersect -sorted -g $GENOME -a $BED_FILE -b {} | wc -l"
#ls $DIR/*gz | xargs -I {} bash -c "bedtools intersect -sorted -g $GENOME -a $BED_FILE -b {} | wc -l"
bedtools intersect -sorted -g $GENOME -a $BED_FILE -b  $DIR/*gz | wc -l
