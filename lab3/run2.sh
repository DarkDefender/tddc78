#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n ,Cores: $1" >> result1.txt
	echo "Iteration: $n ,Cores: $1" >> result2.txt
	echo "Iteration: $n ,Cores: $1" >> result3.txt
	echo "Iteration: $n ,Cores: $1" >> result4.txt
	OMP_NUM_THREADS=$1 ./test1.out >> result1.txt
	OMP_NUM_THREADS=$1 ./test2.out >> result2.txt
	OMP_NUM_THREADS=$1 ./test3.out >> result3.txt
	OMP_NUM_THREADS=$1 ./test4.out >> result4.txt
done

