#!/bin/bash
set -o nounset

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$#" -ne "2" ]
then
    echo "useage: $0 <db> <bed>"
    exit
fi

DB=$1
BED_FILE=$2

BASE=`basename $BED_FILE`
MD5=`md5 -q $BED_FILE`

SQL_FILE=$BASE.$MD5

if [ ! -f $SQL_FILE ]; then
IFS=$'\n'
for Q in `cat $BED_FILE | cut -f1,2,3`;do
    CHRM=`echo -e "$Q" | cut -f1`
    BEG=`echo -e "$Q" | cut -f2`
    END=`echo -e "$Q" | cut -f3`
    ${DIR}/reg2query $CHRM $BEG $END >> $SQL_FILE
#    BINS=`$DIR/reg2bins $BEG $END`
#    Q="SELECT A.NAME, COUNT(B.ROWID)
#    FROM
#        FILES A
#        LEFT JOIN
#        ( SELECT FILE_ID,ROWID FROM INTERVALS 
#            WHERE CHRM = \"$CHRM\" 
#                AND BIN IN ($BINS) 
#                AND END>=$BEG 
#                AND START<=$END 
#        ) B
#    ON A.ROWID = B.FILE_ID
#    GROUP BY A.NAME;"
#    echo "$Q" >> $SQL_FILE
done
fi

/usr/bin/time sqlite3 $DB  < $SQL_FILE > /dev/null
#rm $SQL_FILE
