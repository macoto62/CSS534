#!/bin/sh

if [ -z "$6" ]; then
       echo "Error: \$1 should be rfc or blood and 5 keywords must be given."
       exit 1
fi

echo "$1 $2 $3 $4 $5 $6"

rm part-00000
hdfs dfs -mkdir /user/chihl4/input
hdfs dfs -copyFromLocal ./$1/* /user/chihl4/input

start_time=$(date +%s%N)

yarn jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -cmdenv key0=$2 -cmdenv key1=$3 -cmdenv key2=$4 -cmdenv key3=$5 -cmdenv key4=$6 -input /user/chihl4/input/ -output /user/chihl4/outputs
#hadoop jar $HADOOP_HOME/share/hadoop/tools/lib/hadoop-streaming-3.4.1.jar -files ./mapper,./reducer -mapper ./mapper -reducer ./reducer -cmdenv key0=$2 -cmdenv key1=$3 -cmdenv key2=$4 -cmdenv key3=$5 -cmdenv key4=$6 -input /user/chihl4/input/ -output /user/chihl4/outputs

end_time=$(date +%s%N)
elapsed_time=$(( (end_time - start_time) / 1000000 ))
echo "Elapsed time = ${elapsed_time} ms"
hdfs dfs -copyToLocal /user/chihl4/outputs/part-00000 part-00000
cat part-00000
