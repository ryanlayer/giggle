[![Build Status](https://travis-ci.org/ryanlayer/giggle.svg?branch=master)](https://travis-ci.org/ryanlayer/giggle)

# GIGGLE

GIGGLE is a genomics search engine that identifies and ranks the
significance of shared genomic loci between query features and thousands of
genome interval files.

For questions and discussion about GIGGLE please visit/join the mailing list:
https://groups.google.com/d/forum/giggle-discuss

## Building

### Dependencies
From a fresh install of Ubuntu, the following steps should provide all the
required dependencies.

    sudo apt install gcc make autoconf zlib1g-dev libbz2-dev libcurl4-openssl-dev libssl-dev ruby
    
### Giggle command line intervace

    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    make
    export GIGGLE_ROOT=`pwd`

### Run tests

    cd test/func
    ./giggle_tests.sh
    cd ../unit
    make
    cd ../../..

### Web server (optional)
This is based on [libmicrohttpd](http://www.gnu.org/software/libmicrohttpd/)

    mkdir -p $HOME/usr/local/
    wget http://ftpmirror.gnu.org/libmicrohttpd/libmicrohttpd-0.9.46.tar.gz
    tar zxvf libmicrohttpd-0.9.46.tar.gz
    cd libmicrohttpd-0.9.46
    ./configure --prefix=$HOME/usr/local/
    make
    make install

    export LD_LIBRARY_PATH=$HOME/usr/local/lib/

    cd ..

    sudo apt install libtool

    wget https://github.com/json-c/json-c/archive/json-c-0.12.1-20160607.tar.gz
    tar xvf json-c-0.12.1-20160607.tar.gz  
    cd json-c-json-c-0.12.1-20160607
    ./configure --prefix=$HOME/usr/local/
    make
    make install

    cd giggle
    make
    make server

## Example analysis

**NOTE:** Index files and query files MUST be bgzipped (https://github.com/samtools/htslib, https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3042176/).

### Roadmap Epigenomics

    # details of how to recreate the data at 
    # https://github.com/ryanlayer/giggle/blob/master/examples/rme/README.md
    wget https://s3.amazonaws.com/layerlab/giggle/roadmap/roadmap_sort.tar.gz
    tar -zxvf roadmap_sort.tar.gz
    
    # NOTE, if the following command gives "Too many open files" try:
    # ulimit -Sn 16384
    $GIGGLE_ROOT/bin/giggle index -s -f \
        -i "roadmap_sort/*gz" \
        -o roadmap_sort_b 

    wget ftp://ftp.ncbi.nlm.nih.gov/geo/samples/GSM1218nnn/GSM1218850/suppl/GSM1218850_MB135DMMD.peak.txt.gz
    # take the just the top peaks
    zcat GSM1218850_MB135DMMD.peak.txt.gz \
    | awk '$8>100' \
    | cut -f1,2,3 \
    | $GIGGLE_ROOT/lib/htslib/bgzip -c \
    > GSM1218850_MB135DMMD.peak.q100.bed.gz

    # List files in the index
    $GIGGLE_ROOT/bin/giggle search -l \
        -i roadmap_sort_b/ 

    # Search
    $GIGGLE_ROOT/bin/giggle search -s \
        -i roadmap_sort_b/ \
        -q GSM1218850_MB135DMMD.peak.q100.bed.gz \
    > GSM1218850_MB135DMMD.peak.q100.bed.gz.result

    
    # Plot
    sudo apt install python python-pip python-tk
    pip install matplotlib
    $GIGGLE_ROOT/scripts/giggle_heat_map.py \
        -s $GIGGLE_ROOT/examples/rme/states.txt \
        -c $GIGGLE_ROOT/examples/rme/EDACC_NAME.txt \
        -i GSM1218850_MB135DMMD.peak.q100.bed.gz.result \
        -o GSM1218850_MB135DMMD.peak.q100.bed.gz.result.3x11.pdf \
        -n $GIGGLE_ROOT/examples/rme/new_groups.txt \
        --x_size 3 \
        --y_size 11 \
        --stat combo \
        --ablines 15,26,31,43,52,60,72,82,87,89,93,101,103,116,120,122,127 \
        --state_names $GIGGLE_ROOT/examples/rme/short_states.txt \
        --group_names $GIGGLE_ROOT/examples/rme/new_groups_names.txt

### Roadmap webserver 

    giggle/bin/server_enrichment roadmap_sort_b/ /tmp/ giggle/examples/rme/data_def.json 8080 


