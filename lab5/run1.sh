#!/bin/bash

cores=1
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test1.sh $1 1 1 result4.txt

cores=2
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test1.sh $1 1 2 result4.txt

cores=4
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test1.sh $1 2 2 result4.txt

cores=8
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test1.sh $1 2 4 result4.txt

cores=16
echo "Cores: $cores" >> result4.txt
salloc -N1 -n$cores ./test1.sh $1 4 4 result4.txt
