# GIGGLE

[![Build Status](https://travis-ci.org/ryanlayer/giggle.svg?branch=master)](https://travis-ci.org/ryanlayer/giggle)


# Epigenomics Roadmap

    mkdir data
    cd data
    wget http://egg2.wustl.edu/roadmap/data/byFileType/chromhmmSegmentations/ChmmModels/coreMarks/jointModel/final/all.mnemonics.bedFiles.tgz
    tar zxvf all.mnemonics.bedFiles.tgz
    cd ..
    mkdir split
    python rename.py states.txt EDACC_NAME.txt "data/*gz" "split/"
    cd split
    ls *.bed | xargs -I {} -P 10 sh -c "bgzip {}"
    ls *.bed.gz | xargs -I {} -P 10 sh -c "tabix {}"
    

##bedtools
###unsorted
    time ls split/*gz | \
        xargs -I{} \
        sh -c "bedtools intersect -a inkids_rare.bed.gz -b {} | wc -l"

    real    2m15.021s
    user    1m57.950s
    sys 0m16.163s

###sorted
    time ls split/*gz | \
        xargs -I{} \
        sh -c "bedtools intersect -sorted -g genome.txt -a inkids_rare.bed.gz -b {} | wc -l"

    real    0m58.092s
    user    0m51.030s
    sys 0m8.076s

##tabix
###index
    time ls *gz | xargs -I{} tabix {}

    real    1m54.741s
    user    1m41.347s
    sys 0m7.689s

###search
    time ls split/*gz | xargs -I{} sh -c "tabix -p bed -B {}  inkids_rare.bed.gz | wc -l"

    real    0m26.832s
    user    0m19.550s
    sys 0m7.597s

##giggle
###index
    time ~/src/giggle/bin/giggle index \
        -i "split/*gz" -o split_i -f
    Indexed 55605005 intervals.

    real    7m59.121s
    user    7m3.515s
    sys 0m41.202s
###just the counts
    time ~/src/giggle/bin/giggle search -c \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m0.727s
    user    0m0.031s
    sys 0m0.677s

###counts and significance tests
    time ~/src/giggle/bin/giggle search -s \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m0.851s
    user    0m0.052s
    sys 0m0.712s   

###all matching records in all files
    time ~/src/giggle/bin/giggle search -v \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m3.249s
    user    0m2.280s
    sys 0m0.822s
