[![Build Status](https://travis-ci.org/ryanlayer/giggle.svg?branch=master)](https://travis-ci.org/ryanlayer/giggle)

# GIGGLE

# Build
    
## Giggle command line intervace

    git clone https://github.com/samtools/htslib.git
    cd htslib
    make
    cd .. 
    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    make

## Web server (optional)
This is based on [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)

    wget http://ftpmirror.gnu.org/libmicrohttpd/libmicrohttpd-0.9.46.tar.gz
    tar zxvf libmicrohttpd-0.9.46.tar.gz
    cd libmicrohttpd-0.9.46
    ./configure --prefix=$HOME/usr/local/
    make
    make install

    git clone https://github.com/json-c/json-c.git
    cd json-c
    sh autogen.sh
    ./configure --prefix=$HOME/usr/local/
    make
    make install

    cd giggle
    make
    make server

# [Dev Docs](http://ryanlayer.github.io/giggle/)

# Example analysis
## GTEx (in examples/gtex)

    mkdir data
    cd data

    wget http://www.gtexportal.org/static/datasets/gtex_analysis_v6/annotations/GTEx_Data_V6_Annotations_SampleAttributesDS.txt

    tail -n+2 GTEx_Data_V6_Annotations_SampleAttributesDS.txt \
    | cut -f 1,6 \
    | sed -e "s/ /_/" \
    > SAMPID_to_SMTS.txt

    wget http://www.gtexportal.org/static/datasets/gtex_analysis_v6/reference/gencode.v19.genes.patched_contigs.gtf.gz
    
    gunzip -c gencode.v19.genes.patched_contigs.gtf.gz \
    | awk '$3=="transcript"' \
    | cut -f1,4,5,9 \
    | sed -e "s/gene_id \"//" \
    | sed -e "s/\".*gene_name \"/\t/" \
    | sed -e "s/\".*$//" \
    > genes.bed

    wget http://www.gtexportal.org/static/datasets/gtex_analysis_v6/rna_seq_data/GTEx_Analysis_v6_RNA-seq_RNA-SeQCv1.1.8_gene_rpkm.gct.gz

    gunzip -c GTEx_Analysis_v6_RNA-seq_RNA-SeQCv1.1.8_gene_rpkm.gct.gz \
    | python one_per_tissue.py

    mv *.bed gtex/.
    cd gtex
    ls | xargs -P 20 -I {} sh -c "bgzip {}"
    cd ..
    ~/src/giggle/bin/giggle index -i "gtex/*gz" -o gtex_b -f


## Epigenomics Roadmap (in examples/rme)

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

## Timings

###bedtools
####unsorted

    time ls split/*gz | \
        xargs -I{} \
        sh -c "bedtools intersect -a inkids_rare.bed.gz -b {} | wc -l"

    real    2m15.021s
    user    1m57.950s
    sys 0m16.163s

####sorted

    time ls split/*gz | \
        xargs -I{} \
        sh -c "bedtools intersect -sorted -g genome.txt -a inkids_rare.bed.gz -b {} | wc -l"

    real    0m58.092s
    user    0m51.030s
    sys 0m8.076s

###tabix
####index

    time ls *gz | xargs -I{} tabix {}

    real    1m54.741s
    user    1m41.347s
    sys 0m7.689s

####search

    time ls split/*gz | xargs -I{} sh -c "tabix -p bed -B {}  inkids_rare.bed.gz | wc -l"

    real    0m26.832s
    user    0m19.550s
    sys 0m7.597s

###giggle
####index

    time ~/src/giggle/bin/giggle index \
        -i "split/*gz" -o split_i -f
    Indexed 55605005 intervals.

    real    7m59.121s
    user    7m3.515s
    sys 0m41.202s

####just the counts

    time ~/src/giggle/bin/giggle search -c \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m0.727s
    user    0m0.031s
    sys 0m0.677s

####counts and significance tests

    time ~/src/giggle/bin/giggle search -s \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m0.851s
    user    0m0.052s
    sys 0m0.712s   

####all matching records in all files

    time ~/src/giggle/bin/giggle search -v \
        -q inkids_rare.bed.gz \
        -i split_i 

    real    0m3.249s
    user    0m2.280s
    sys 0m0.822s

## Epigenomics Roadmap Server

    ~/src/giggle/bin/server_overlap \
        1 \
        split_i \
        ~/src/giggle/examples/rme/web/track_names.txt \
        ~/src/giggle/examples/rme/web/header.txt

Then open `~src/giggle/scripts/get_overlaps.html`
