[![Build Status](https://travis-ci.org/ryanlayer/giggle.svg?branch=master)](https://travis-ci.org/ryanlayer/giggle)

# GIGGLE

GIGGLE is a genomics search engine that identifies and ranks the
significance of shared genomic loci between query features and thousands of
genome interval files.

For questions and discussion about GIGGLE please visit/join the mailing list:
https://groups.google.com/d/forum/giggle-discuss

For more information about how GIGGLE works, please read the preprint:
http://www.biorxiv.org/content/early/2017/06/29/157735

## Building

### Dependencies
From a fresh install of Ubuntu, the following steps should provide all the
required dependencies.

    sudo apt install gcc make autoconf zlib1g-dev libbz2-dev libcurl4-openssl-dev libssl-dev ruby
    
### Giggle command line interface

    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    make
    export GIGGLE_ROOT=`pwd`
    cd ..

### Run tests

The first set of tests require bedtools to be in your path.

    sudo apt install g++ python

    git clone https://github.com/arq5x/bedtools2.git
    cd bedtools2
    make
    cd bin
    export PATH=$PATH:`pwd`
    cd ../..
    
Now run the tests

    cd $GIGGLE_ROOT/test/func
    ./giggle_tests.sh
    cd ../unit
    make
    cd ../../..
    
### Hosted data and services

#### Data
Roadmap Epigenomics:  https://s3.amazonaws.com/layerlab/giggle/roadmap/roadmap_sort.tar.gz

UCSC Genome browser:  https://s3.amazonaws.com/layerlab/giggle/ucsc/ucscweb_sort.tar.gz

Fantom5:  https://s3.amazonaws.com/layerlab/giggle/fantom/fantom_sort.tar.gz

#### Interactive heatmap

http://ryanlayer.github.io/giggle/index.html?primary_index=ec2-54-227-176-15.compute-1.amazonaws.com/rme&ucsc_index=ec2-54-227-176-15.compute-1.amazonaws.com/ucsc

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

    cd $GIGGLE_ROOT
    make
    make server
    cd ..
    
To host the site shown in Supplemental Figure 3, you will need host web servers for both 
the Roadmap Epigenomics data and the UCSC data. Here we will run both servers on the same
host from ports `8080` and `8081` and access the web services using `localhost`, but these 
are generall steps and apply to many other configurations including hosting the data sets 
on different servers.
     
    wget https://s3.amazonaws.com/layerlab/giggle/roadmap/roadmap_sort.tar.gz
    tar -zxvf roadmap_sort.tar.gz
    
    # NOTE, if the following command gives "Too many open files" try:
    ulimit -Sn 16384
    $GIGGLE_ROOT/bin/giggle index -s -f \
        -i "roadmap_sort/*gz" \
        -o roadmap_sort_b 
        
    wget https://s3.amazonaws.com/layerlab/giggle/ucsc/ucscweb_sort.tar.gz
    tar -zxvf ucscweb_sort.tar.gz
    
    $GIGGLE_ROOT/bin/giggle index -s -f \
        -i "ucscweb_sort/*gz" \
        -o ucscweb_sort_b
    
Start a web server for each index. 

    $GIGGLE_ROOT/bin/server_enrichment roadmap_sort_b/ /tmp/ $GIGGLE_ROOT/examples/rme/data_def.json 8080 &
    $GIGGLE_ROOT/bin/server_enrichment ucscweb_sort_b/ /tmp/ $GIGGLE_ROOT/examples/ucsc/data_def.json 8081 &

Pass these two services to the web interface through URL arguments:

    http://ryanlayer.github.io/giggle/index.html?primary_index=localhost:8080&ucsc_index=localhost:8081
   
These data are also being served here:

http://ryanlayer.github.io/giggle/index.html?primary_index=ec2-54-227-176-15.compute-1.amazonaws.com/rme&ucsc_index=ec2-54-227-176-15.compute-1.amazonaws.com/ucsc

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


## APIs

### [Python](https://github.com/brentp/python-giggle) by Brent Pedersen

```
from giggle import Giggle
index = Giggle('existing-index-dir') # or Giggle.create('new-index-dir', 'files/*.bed')
print(index.files)

result = index.query('chr1', 9999, 20000)
print(result.n_files)
print(result.n_total_hits) # integer number sum of hits across all files

print(result.n_hits(0)) # integer number of hits for the 0th file...

for hit in result[0]:
    print(hit) # hit is a string
```

#### Installation

make sure you have `liz`, `libcurl`, `libcrypto`, `libbz2` and `liblzma` installed in the appropriate
place on your system.
```
git clone --recursive https://github.com/brentp/python-giggle
cd python-giggle
python setup.py test
python setup.py install
```

### [Go](https://github.com/brentp/go-giggle) by Brent Pedersen

[![GoDoc](https://godoc.org/github.com/brentp/go-giggle?status.png)](https://godoc.org/github.com/brentp/go-giggle)

```Go

import (
    giggle "github.com/brentp/go-giggle"
    "fmt"
) 

func main() {

    index := giggle.Open("/path/to/index")
    res := index.Query("1", 565657, 567999)

    // all files in the index
    index.Files()

    // int showing total count
    res.TotalHits()

    // []uint32 giving number of hits for each file
    res.Hits()

    var lines []string
    # access results by index of file.
    lines = res.Of(0)
    fmt.Println(strings.Join(lines, "\n"))
    lines = res.Of(1)
}
```
