#!/bin/bash
set -o nounset

if [ "$#" -ne "4" ]
then
    echo "useage: $0 <db> <chr> <start> <end>"
    exit
fi

DB=$1
CHRM=$2
START=$3
END=$4

#echo "CREATE TABLE FILES ( NAME TEXT );" | sqlite3 $OUTPUT_DB
#echo "CREATE TABLE INTERVALS ( FILE_ID INT, BIN INT, CHRM TEXT, START INT, END INT);" \
#| sqlite3 $OUTPUT_DB

BINS=`./reg2bins $START $END`

#Q="SELECT FILE_ID,COUNT(*) FROM INTERVALS WHERE 
#CHRM = \"$CHRM\" AND 
#BIN IN ($BINS) AND 
#END>=$START AND START<=$END 
#GROUP BY FILE_ID;"

#Q="SELECT FILE_ID,COUNT(*) FROM INTERVALS WHERE 
#CHRM = \"$CHRM\" AND 
#BIN IN ($BINS) AND 
#END>=$START AND START<=$END 
#GROUP BY FILE_ID;"

Q="SELECT A.NAME, COUNT(B.ROWID)
FROM
    FILES A
    LEFT JOIN
    ( SELECT FILE_ID,ROWID FROM INTERVALS 
        WHERE CHRM = \"$CHRM\" 
            AND BIN IN ($BINS) 
            AND END>=$START 
            AND START<=$END 
    ) B
ON A.ROWID = B.FILE_ID
GROUP BY A.NAME"

#Q="SELECT A.NAME, COUNT(B.ROWID)
#FROM
#    FILES A
#    LEFT JOIN
#    ( SELECT FILE_ID,ROWID FROM INTERVALS 
#        WHERE CHRM = \"$CHRM\" 
#            AND END>=$START 
#            AND START<=$END 
#    ) B
#ON A.ROWID = B.FILE_ID
#GROUP BY A.NAME"




echo "$Q"

sqlite3 $DB "$Q"
