#!/bin/bash
set -o nounset

if [ "$#" -ne "2" ]
then
    echo "useage: $0 <dir> <bed>"
    exit
fi

DIR=$1
export BED_FILE=$2


ls $DIR/*gz | xargs -I {} bash -c "tabix -B {} $BED_FILE | wc -l"
