#!/bin/sh                                                                                                                                                                  

if [ -z "$6" ]; then
       echo "Error: \$1 should be rfc or blood and 5 keywords must be given."
       exit 1
fi

echo "$1 $2 $3 $4 $5 $6"

rm $1-part-0000

hdfs dfs -mkdir /user/chihl4/$1
hdfs dfs -copyFromLocal ./$1/* /user/chihl4/$1

#hadoop jar invertedindexes.jar InvertedIndexes $1 output $2 $3 $4 $5 $6
yarn jar invertedindexes.jar InvertedIndexes $1 output $2 $3 $4 $5 $6

rm -f $1-part-00000
hdfs dfs -copyToLocal /user/chihl4/output/part-00000 $1-part-00000
