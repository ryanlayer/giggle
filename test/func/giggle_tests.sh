#!/bin/bash

test -e ssshtest || wget -q https://raw.githubusercontent.com/ryanlayer/ssshtest/master/ssshtest

. ssshtest

STOP_ON_FAIL=0

BEDTOOLS=`which bedtools`

echo $BEDTOOLS


# Make the index
../../bin/giggle index \
    -i "../data/many/*gz" \
    -o ../data/many_i \
    -f \
2> /dev/null

../../bin/giggle index \
    -i "../data/chr_mix/*gz" \
    -o ../data/chr_mix_i \
    -f \
2> /dev/null


if [ -n "$BEDTOOLS" ]
then
    rm -f bt.out
    ls ../data/many/*gz \
    | xargs -I{} \
        sh -c \
        "$BEDTOOLS intersect -wa -b ../data/1k.sort.bed.gz -a {} >> bt.out"

    run check_intersections_per_file \
        ../../bin/giggle search \
        -q ../data/1k.sort.bed.gz \
        -i ../data/many_i \
        -v 
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
        ../../bin/giggle search \
        -i ../data/chr_mix_i \
        -r $R_CHRM:$R_START-$R_END 
        
assert_equal 0 $(diff $STDOUT_FILE <( ../../bin/giggle search \
                                        -i ../data/chr_mix_i \
                                        -r chr$R_CHRM:$R_START-$R_END) | wc -l)
done
