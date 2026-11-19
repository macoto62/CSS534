#!/bin/sh

rm part-00000
hdfs dfs -mkdir /user/css534/input
hdfs dfs -copyFromLocal ./test.txt /user/css534/input

echo "start time ============================="
date +"%s.%N"

yarn jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -input /user/css534/input/test.txt -output /user/css534/outputs
#hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -input /user/css534/input/test.txt -output /user/css534/outputs

echo "end time ==============================="
date +"%s.%N"

hdfs dfs -copyToLocal /user/css534/outputs/part-00000 part-00000
cat part-00000
