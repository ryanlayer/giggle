#!/bin/bash
set -o nounset


if [ "$#" -ne "2" ]
then
    echo "useage: $0 <dir> <bed>"
    exit
fi

DIR=$1
export BED_FILE=$2
export TABIX=


ls $DIR/*gz | xargs -I {} bash -c "$TABIX -r $BED_FILE {} | wc -l"
