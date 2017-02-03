Get files
    git clone https://github.com/arq5x/bedtools2.git
    cd bedtools2 
    git checkout 61143078ebe6194689c8542e13d668480592b830
    make
    BEDTOOLS_ROOT=`pwd`
    cd ..

    git clone https://github.com/samtools/htslib.git
    cd htslib
    git checkout d8d0323bca21cfd131e2f183d12c9dd31a5ed75b
    make
    HTSLIB_ROOT=`pwd`
    cd ..

    git clone https://github.com/ryanlayer/giggle.git
    cd giggle
    git checkout a4d826e2a739ebad77cb688a9f61b3c0e75638d5
    make
    GIGGLE_ROOT=`pwd`

Roadmap database with subsets for testing


    mkdir data
    cd data
    wget http://egg2.wustl.edu/roadmap/data/byFileType/chromhmmSegmentations/ChmmModels/coreMarks/jointModel/final/all.mnemonics.bedFiles.tgz
    mkdir orig
    tar zxvf all.mnemonics.bedFiles.tgz -C orig/
    mkdir split

    pip install toolshed

    python $GIGGLE_ROOT/examples/rme/rename.py \
        $GIGGLE_ROOT/examples/rme/states.txt \
        $GIGGLE_ROOT/examples/rme/EDACC_NAME.txt \
        "orig/*gz" \
        "split/"

    cd split
    ls *.bed | xargs -I {} -P 10 bash -c "bgzip {}"
    cd ..

    $GIGGLE_ROOT/scripts/sort_bed "split/*gz" split_sort/ 30

    cd split_sort
    time ls *.bed.gz | xargs -I {} bash -c "tabix {}"

    real    2m12.584s
    user    1m27.827s
    sys     0m22.229s

    cd ..

    time $GIGGLE_ROOT/bin/giggle index \
        -i "split_sort/*gz" \
        -o split_sort_b \
        -f -s
    Indexed 55605005 intervals.

    real    1m22.409s
    user    1m18.907s
    sys     0m3.379s

    SIZE=`ls -l split_sort/*gz | awk 'BEGIN {sum=0} {sum += $5;} END {print sum}'`

    ls -l split_sort/*gz \
    | awk '{OFS="\t"; print rand(),$5,$9;}' \
    | sort -g \
    | awk -v t=$SIZE '
        BEGIN {   batch_size=0; batch_id=0 }
        {
            OFS="\t";
            if (size < t/4) {
                size += $2
            } else {
                print size;
                size = $2
                batch_id += 1;
            }
            print batch_id,$2,$3;
        }'\
    > batches_sizes.txt

    # 1 -> 1/2, 3 full data set

    mkdir split_sort_s1
    cd split_sort_s1
    cat ../batches_sizes.txt \
        | awk '$1<=1' \
        | cut -f3 \
        | xargs -I{} bash -c "ln -s ../{} .;ln -s ../{}.tbi"   
    cd ..

    time $GIGGLE_ROOT/bin/giggle index \
        -i "split_sort_s1/*gz" \
        -o split_sort_s1_b \
        -f -s
    Indexed 27858577 intervals.

    real    0m38.561s
    user    0m36.584s
    sys     0m1.946s

    ln -s split_sort split_sort_s3
    ln -s split_sort_b split_sort_b_s3

    cd ..

Random query sets

    cd data

    zcat split/Adipose_Nuclei_Repressed_PolyComb.bed.gz \
    | cut -f1 \
    | uniq > rme_chrm_order.txt

    for s in `cat rme_chrm_order.txt`; do
        grep -w $s $GIGGLE_ROOT/data/human.hg19.genome
    done \
    > rme.human.hg19.genome

    bedtools random -n 10 -g rme.human.hg19.genome  > r10.bed
    bedtools random -n 100 -g rme.human.hg19.genome  > r100.bed
    bedtools random -n 1000 -g rme.human.hg19.genome  > r1000.bed
    bedtools random -n 10000 -g rme.human.hg19.genome  > r10000.bed
    bedtools random -n 100000 -g rme.human.hg19.genome  > r100000.bed
    bedtools random -n 1000000 -g rme.human.hg19.genome  > r1000000.bed
    
    
    #wget -O gsort https://github.com/brentp/gsort/releases/download/v0.0.4/gsort_darwin_amd64
    #wget -O gsort https://github.com/brentp/gsort/releases/download/v0.0.4/gsort_linux_amd64

    chmod +x gsort

    ./gsort r10.bed rme.human.hg19.genome > r10.sort.bed
    ./gsort r100.bed rme.human.hg19.genome > r100.sort.bed
    ./gsort r1000.bed rme.human.hg19.genome > r1000.sort.bed
    ./gsort r10000.bed rme.human.hg19.genome > r10000.sort.bed
    ./gsort r100000.bed rme.human.hg19.genome > r100000.sort.bed
    ./gsort r1000000.bed rme.human.hg19.genome > r1000000.sort.bed

    bgzip -c r10.sort.bed > r10.sort.bed.gz
    bgzip -c r100.sort.bed > r100.sort.bed.gz
    bgzip -c r1000.sort.bed > r1000.sort.bed.gz
    bgzip -c r10000.sort.bed > r10000.sort.bed.gz
    bgzip -c r100000.sort.bed > r100000.sort.bed.gz
    bgzip -c r1000000.sort.bed > r1000000.sort.bed.gz

    cd ..

SQLITE / UCSC 

    cd sqlite

    gcc -o ucsc_idx ucsc_idx.c
    gcc -o reg2query reg2query.c
    ./ucsc_build.sh ../data/split_sort_s1 ucsc_s1.db
    ./ucsc_build.sh ../data/split_sort_s3 ucsc_s3.db

    cd ..

Speed tests
    
    for database in 1 3; do
        for query in 10 100 1000 10000 100000 1000000; do
            ./speed_test.sh $query $database
        done
    done
