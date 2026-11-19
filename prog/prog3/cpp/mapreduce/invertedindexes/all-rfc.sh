#!/bin/sh

if [ -z "$1" ]; then
       echo "Error: \$1: workers"
       exit 1
fi

echo "$1"

$HADOOP_HOME/rm-hadoop-tmp-files.sh $HADOOP_HOME/etc/hadoop/$1
$HADOOP_HOME/reformat.sh
./run.sh rfc TCP UDP LAN PPP HDLC
stop-all.sh
