#!/bin/bash

GIGGLE=

if [ "$#" -ne "2" ]; then
    echo "usage: $0 <random size> <subset num>"
    exit 1
fi

R=$1
S=$2

echo -n "bedtools $R $S "
if [ "$(uname)" == "Darwin" ]; then
	/usr/bin/time bedtools/bedtools_many_search.sh data/split_sort_s$S/ data/r$R.sort.bed  data/rme.human.hg19.genome > /dev/null
else
	/usr/bin/time -f "%e real\t%U user\t%S sys" bedtools/bedtools_many_search.sh data/split_sort_s$S/ data/r$R.sort.bed  data/rme.human.hg19.genome > /dev/null
fi

echo -n "tabix $R $S "
if [ "$(uname)" == "Darwin" ]; then
	/usr/bin/time tabix/tabix_many_search.sh data/split_sort_s$S/ data/r$R.sort.bed > /dev/null
else
	/usr/bin/time -f "%e real\t%U user\t%S sys" tabix/tabix_many_search.sh data/split_sort_s$S/ data/r$R.sort.bed > /dev/null
fi

echo -n "sqlite3 $R $S "
sqlite/ucsc_many_search.sh sqlite/ucsc_s$S.db data/r$R.sort.bed

echo -n "giggle $R $S "
if [ "$(uname)" == "Darwin" ]; then
	/usr/bin/time $GIGGLE search -i data/split_sort_s$S\_b/ -q data/r$R.sort.bed.gz > /dev/null
else
	/usr/bin/time -f "%e real\t%U user\t%S sys" $GIGGLE search -i data/split_sort_s$S\_b/ -q data/r$R.sort.bed.gz > /dev/null
fi
