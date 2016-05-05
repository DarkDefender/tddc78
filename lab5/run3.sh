#!/bin/bash

cores=2
echo "Cores: $cores" >> result1.txt
salloc -N1 -n$cores ./test3.sh $1 1 2 5000 10000 result1.txt

cores=4
echo "Cores: $cores" >> result2.txt
salloc -N1 -n$cores ./test3.sh $1 2 2 10000 10000 result2.txt

cores=8
echo "Cores: $cores" >> result3.txt
salloc -N1 -n$cores ./test3.sh $1 2 4 50000 10000 result3.txt

cores=16
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test3.sh $1 4 4 100000 10000 result4.txt
