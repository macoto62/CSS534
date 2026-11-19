#!/bin/sh

rm part-00000
hdfs dfs -mkdir /user/chihl4/input
hdfs dfs -copyFromLocal ./test.txt /user/chihl4/input

echo "start time ============================="
date +"%s.%N"

yarn jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -input /user/chihl4/input/test.txt -output /user/chihl4/outputs
#hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -input /user/chihl4/input/test.txt -output /user/chihl4/outputs

echo "end time ==============================="
date +"%s.%N"

hdfs dfs -copyToLocal /user/chihl4/outputs/part-00000 part-00000
cat part-00000
