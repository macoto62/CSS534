#!/bin/sh                                                                                                                                                                  

rm part-00000
hdfs dfs -mkdir /user/css534/input
hdfs dfs -copyFromLocal ./input/* /user/css534/input

echo "start time ============================="
date +"%s.%N"

#hadoop jar wordcount.jar WordCount input output
yarn jar wordcount.jar WordCount input output

echo "end time ==============================="
date +"%s.%N"

hdfs dfs -copyToLocal /user/css534/output/part-r-00000 ./output/part-r-00000
cat ./output/part-r-00000
