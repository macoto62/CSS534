#!/bin/sh

javac -classpath $HADOOP_CLASSPATH InvertedIndexes.java
jar -cvf invertedindexes.jar *.class
