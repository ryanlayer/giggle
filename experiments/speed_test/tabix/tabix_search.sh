#!/bin/bash
set -o nounset

if [ "$#" -ne "4" ]
then
    echo "useage: $0 <dir> <chr> <start> <end>"
    exit
fi

DIR=$1
CHRM=$2
START=$3
END=$4


ls $DIR/*gz | xargs -I {} bash -c "tabix {} $CHRM:$START-$END | wc -l"
