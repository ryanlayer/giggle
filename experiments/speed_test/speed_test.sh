#!/bin/bash

if [ "$#" -ne "2" ]; then
    echo "usage: $0 <random size> <subset num>"
    exit 1
fi


R=$1
S=$2

echo -n "bedtools $R $S "
/usr/bin/time bedtools/bedtools_many_search.sh data/split_s$S/ data/r$R.sort.bed  data/rme.human.hg19.genome > /dev/null
echo -n "tabix $R $S "
/usr/bin/time tabix/tabix_many_search.sh data/split_s$S/ data/r$R.sort.bed > /dev/null
echo -n "sqlite3 $R $S "
sqlite/ucsc_many_search.sh sqlite/ucsc_s$S.db data/r$R.sort.bed
echo -n "giggle $R $S "
/usr/bin/time giggle search -i data/split_s$S\_b/ -q data/r$R.sort.bed.gz > /dev/null
