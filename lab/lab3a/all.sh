#!/bin/sh

if [ -z "$1" ]; then
       echo "Error: \$1 should be a workers file"
       exit 1
fi

$HADOOP_HOME/rm-hadoop-tmp-files.sh $HADOOP_HOME/etc/hadoop/$1
$HADOOP_HOME/reformat.sh
./run.sh
stop-all.sh
