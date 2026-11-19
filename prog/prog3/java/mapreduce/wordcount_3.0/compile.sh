#!/bin/sh

javac -classpath $HADOOP_CLASSPATH WordCount.java
jar -cvf wordcount.jar *.class
