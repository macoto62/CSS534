#!/bin/sh                                                                                                                                                                  

rm part-00000
rm ./output/*
hdfs dfs -mkdir /user/chihl4/input
hdfs dfs -copyFromLocal ./input/* /user/chihl4/input

echo "start time ============================="
date +"%s.%N"

#hadoop jar wordcount.jar WordCount input output
yarn jar wordcount.jar WordCount input output

echo "end time ==============================="
date +"%s.%N"

hdfs dfs -copyToLocal /user/chihl4/output/part-r-00000 ./output/part-r-00000
cat ./output/part-r-00000
