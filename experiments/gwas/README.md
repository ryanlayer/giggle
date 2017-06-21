Get source code

    git clone https://github.com/samtools/htslib.git
    cd htslib
    autoheader
    autoconf
    ./configure --disable-bz2 --disable-lzma --enable-libcurl
    make -j 20
    export HTSLIB_ROOT=`pwd`
    cd ..

    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    make
    export GIGGLE_ROOT=`pwd`
    cd ..

    wget -O gargs https://github.com/brentp/gargs/releases/download/v0.3.6/gargs_linux
    chmod +x gargs


Get database data and index

    mkdir data
    cd data
    wget http://egg2.wustl.edu/roadmap/data/byFileType/chromhmmSegmentations/ChmmModels/coreMarks/jointModel/final/all.mnemonics.bedFiles.tgz
    mkdir orig
    tar zxvf all.mnemonics.bedFiles.tgz -C orig/
    mkdir split

    pip install toolshed --user

    python $GIGGLE_ROOT/examples/rme/rename.py \
        $GIGGLE_ROOT/examples/rme/states.txt \
        $GIGGLE_ROOT/examples/rme/EDACC_NAME.txt \
        "orig/*gz" \
        "split/"

    cd split
    ls *.bed | ../gargs -p 30 "bgzip {}"
    cd ..

    mkdir split_sort

    $GIGGLE_ROOT/scripts/sort_bed "split/*gz" split_sort/ 30

    time $GIGGLE_ROOT/bin/giggle index \
        -i "split_sort/*gz" \
        -o split_sort_b \
        -f -s
    Indexed 55605005 intervals.

    real    1m19.609s
    user    1m16.027s
    sys     0m3.127s

Run experiment

    wget http://hgdownload.cse.ucsc.edu/goldenpath/hg19/database/gwasCatalog.txt.gz

    gunzip -c gwasCatalog.txt.gz \
    | grep "Rheumatoid arthritis" \
    | cut -f2,3,4,18 \
    | grep -v "_" \
    | awk '$4<5E-10' \
    | cut -f1,2,3 \
    | sort -u \
    | bgzip -c \
    > RA.bed.gz

   $GIGGLE_ROOT/bin/giggle search \
    -i rme_data/split_sort_b \
    -q RA.bed.gz \
    -s \
    > RA.bed.gz.giggle.result

    $GIGGLE_ROOT/scripts/giggle_heat_map.py \
        -s $GIGGLE_ROOT/examples/rme/states.txt \
        --state_names $GIGGLE_ROOT/examples/rme/short_states.txt \
        -c $GIGGLE_ROOT/examples/rme/EDACC_NAME.txt \
        -i RA.bed.gz.giggle.result \
        -o RA.bed.gz.giggle.result.3x11.pdf \
        -n $GIGGLE_ROOT/examples/rme/new_groups.txt \
        --x_size 3 \
        --y_size 11 \
        --stat combo \
        --ablines 15,26,31,43,52,60,72,82,87,89,93,101,103,116,120,122,127 \
        --group_names $GIGGLE_ROOT/examples/rme/new_groups_names.txt
        #--no_ylabels \
