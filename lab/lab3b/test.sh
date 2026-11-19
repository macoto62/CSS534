#!/bin/sh

echo "apple banana apple" > test.txt
cat ./test.txt | ./mapper | sort -k1,1 | ./reducer
