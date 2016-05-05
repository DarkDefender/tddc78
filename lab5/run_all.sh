#!/bin/bash

./run1.sh ./part
tar cf part1.tar result*.txt
rm result*.txt
salloc -N1 ./run2.sh ./part
tar cf part2.tar result*.txt
rm result*.txt
./run3.sh ./part
tar cf part3.tar result*.txt
rm result*.txt

exit 0
