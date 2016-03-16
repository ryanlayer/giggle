#!/bin/bash

test -e ssshtest || wget -q https://raw.githubusercontent.com/ryanlayer/ssshtest/master/ssshtest

. ssshtest

STOP_ON_FAIL=0

BEDTOOLS=`which bedtools`


# Make the index
../../bin/index "../data/many/*gz" ../data/many_i i 1 2> /dev/null
../../bin/index "../data/chr_mix/*gz" ../data/chr_mix_i i 1 2> /dev/null


if [ -n "$BEDTOOLS" ]
then
    rm -f bt.out
    ls ../data/many/*gz \
    | xargs -I{} \
        sh -c \
        "$BEDTOOLS intersect -wa -b ../data/1k.sort.bed.gz -a {} >> bt.out"

    run check_intersections_per_file \
        ../../bin/search_file \
        ../data/1k.sort.bed.gz \
        ../data/many_i i 
    assert_exit_code 0
    assert_equal 0 $(diff <(grep -v "#" $STDOUT_FILE | sort) <(cat bt.out | sort) | wc -l)
    rm -f bt.out
fi



for i in `seq 1 10`
do
R_CHRM=$((RANDOM%21 + 1))
R_START=$RANDOM
R_END=$((RANDOM*60+1+R_START))
run check_chr_v_nochr_search_$i \
        ../../bin/search \
        ../data/chr_mix_i \
        $R_CHRM:$R_START-$R_END \
        i
assert_equal 0 $(diff $STDOUT_FILE <( ../../bin/search \
                                        ../data/chr_mix_i \
                                        chr$R_CHRM:$R_START-$R_END \
                                        i) | wc -l)
done
