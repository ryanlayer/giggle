Roadmap database with subsets for testing

    mkdir data
    cd data
    wget http://egg2.wustl.edu/roadmap/data/byFileType/chromhmmSegmentations/ChmmModels/coreMarks/jointModel/final/all.mnemonics.bedFiles.tgz
    mkdir orig
    tar zxvf all.mnemonics.bedFiles.tgz -C orig/
    mkdir split

    pip install toolshed

    python ../../../examples/rme/rename.py \
        ../../../examples/rme/states.txt \
        ../../../examples/rme/EDACC_NAME.txt \
        "orig/*gz" \
        "split/"

    cd split

    ls *.bed | xargs -I {} -P 10 sh -c "bgzip {}"
    ls *.bed.gz | xargs -I {} -P 10 sh -c "tabix {}"
    cd ..
    
    SIZE=`ls -l split/*gz | awk 'BEGIN {sum=0} {sum += $5;} END {print sum}'`

    ls -l split/*gz \
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

    # 0 -> 1/4 the data, 1 -> 1/2, 2 -> 3/4, 3 full data set

    mkdir split_s0
    cd split_s0
    cat ../batches_sizes.txt \
        | awk '$1<=0' \
        | cut -f3 \
        | xargs -I{} bash -c "ln -s ../{} .;ln -s ../{}.tbi"   
    cd ..

    mkdir split_s1
    cd split_s1
    cat ../batches_sizes.txt \
        | awk '$1<=1' \
        | cut -f3 \
        | xargs -I{} bash -c "ln -s ../{} .;ln -s ../{}.tbi"   
    cd ..

    mkdir split_s2
    cd split_s2
    cat ../batches_sizes.txt \
        | awk '$1<=2' \
        | cut -f3 \
        | xargs -I{} bash -c "ln -s ../{} .;ln -s ../{}.tbi"   
    cd ..

    ln -s split split_s3

    cd ..

Random query sets

    cd data

    zcat split/Adipose_Nuclei_Repressed_PolyComb.bed.gz \
    | cut -f1 \
    | uniq > rme_chrm_order.txt

    for s in `cat rme_chrm_order.txt`; do
        grep -w $s ../../../data/human.hg19.genome
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
    ./ucsc_build.sh ../data/split_s0 ucsc_s0.db
    ./ucsc_build.sh ../data/split_s1 ucsc_s1.db
    ./ucsc_build.sh ../data/split_s2 ucsc_s2.db
    ./ucsc_build.sh ../data/split_s3 ucsc_s3.db

    cd ..

GIGGLE

    cd data 

    giggle index -i "split_s0/*gz" -o split_s0_b
    giggle index -i "split_s1/*gz" -o split_s1_b
    giggle index -i "split_s2/*gz" -o split_s2_b
    giggle index -i "split_s3/*gz" -o split_s3_b

    cd ..

Speed tests
    
    for database in 0 1 2 3; do
        for query in 10 100 1000 10000 100000 1000000; do
            ./speed_test.sh $query $database
        done
    done
