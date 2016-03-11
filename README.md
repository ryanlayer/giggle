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
    

    time ls split/*gz | xargs -I{} sh -c "bedtools intersect -a inkids_rare.bed.gz -b {} | wc -l"

    real    2m15.021s
    user    1m57.950s
    sys 0m16.163s


    time ls split/*gz | xargs -I{} sh -c "bedtools intersect -sorted -g genome.txt -a inkids_rare.bed.gz -b {} | wc -l"
    real    0m58.092s
    user    0m51.030s
    sys 0m8.076s

    time ls split/*gz | xargs -I{} sh -c "tabix -p bed -B {}  inkids_rare.bed.gz | wc -l"
    real    0m26.832s
    user    0m19.550s
    sys 0m7.597s

    time ~/src/giggle/bin/search_file inkids_rare.bed.gz split_i/ i | grep "^#"
    real    0m3.249s
    user    0m2.280s
    sys 0m0.822s
