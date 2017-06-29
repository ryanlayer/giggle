# GIGGLE

GIGGLE is a genomics search engine that identifies and ranks the
significance of shared genomic loci between query features and thousands of
genome interval files.

[![Build Status](https://travis-ci.org/ryanlayer/giggle.svg?branch=master)](https://travis-ci.org/ryanlayer/giggle)

# Building

## Dependencies
From a fresh install of Ubuntu, the following steps should provide all the
required dependencies.

    sudo apt install gcc make autoconf zlib1g-dev libbz2-dev libcurl4-openssl-dev libssl-dev ruby
    
## Giggle command line intervace

    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    make
    export GIGGLE_ROOT=`pwd`

## Run tests

    cd test/func
    ./giggle_tests.sh
    cd ../unit
    make
    cd ../../..


## Web server (optional)
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

# Example analysis
## Roadmap Epigenomics

    # details of how to recreate the data at 
    # https://github.com/ryanlayer/giggle/blob/master/examples/rme/README.md
    wget https://s3.amazonaws.com/layerlab/giggle/roadmap/split_sort.tar.gz
    tar -zxvf split_sort.tar.gz
    
    # NOTE, if the following command gives "Too many open file" try running "ulimit -Sn 16384"
    $GIGGLE_ROOT/bin/giggle index -i "split_sort/*gz" -o split_sort_b -s -f

    wget ftp://ftp.ncbi.nlm.nih.gov/geo/samples/GSM1218nnn/GSM1218850/suppl/GSM1218850_MB135DMMD.peak.txt.gz
    # take the just the top peaks
    zcat GSM1218850_MB135DMMD.peak.txt.gz \
    | awk '$8>100' \
    | cut -f1,2,3 \
    | $GIGGLE_ROOT/lib/htslib/bgzip -c \
    > GSM1218850_MB135DMMD.peak.q100.bed.gz

    $GIGGLE_ROOT/bin/giggle search -s \
        -i split_sort_b/ \
        -q GSM1218850_MB135DMMD.peak.bed.gz \
    > GSM1218850_MB135DMMD.peak.bed.gz.result

## Roadmap webserver 

    giggle/bin/server_enrichment split_sort_b/ /tmp/ giggle/examples/rme/data_def.json 8080 


