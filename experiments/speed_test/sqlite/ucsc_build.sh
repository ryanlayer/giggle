#!/bin/bash

if [ "$#" -ne "2" ]
then
    echo "useage: $0 <input dir> <output db>"
    exit
fi

INPUT_DIR=$1
OUTPUT_DB=$2

rm -f $OUTPUT_DB

echo "CREATE TABLE FILES ( NAME TEXT );" | sqlite3 $OUTPUT_DB
echo "CREATE TABLE INTERVALS ( FILE_ID INT, BIN INT, CHRM TEXT, START INT, END INT);" \
| sqlite3 $OUTPUT_DB


for FILE in `ls $INPUT_DIR/*gz`
do
    BASE=`basename $FILE`

    echo $BASE

    echo "INSERT INTO FILES VALUES ( '$BASE');" | sqlite3 $OUTPUT_DB
    FILE_ID=`sqlite3 $OUTPUT_DB "select ROWID from FILES where NAME='$BASE';"`

    SQL_FILE=.UCSC.$RANDOM.$RANDOM

    echo "BEGIN TRANSACTION;" > $SQL_FILE

    ./ucsc_idx $FILE_ID <(zcat < $FILE) >>  $SQL_FILE

    echo "COMMIT TRANSACTION;" >> $SQL_FILE
    
    sqlite3 $OUTPUT_DB < $SQL_FILE

    sqlite3 $OUTPUT_DB  "CREATE INDEX idx1 on INTERVALS(BIN,CHRM,START,END);"

    rm $SQL_FILE
done
