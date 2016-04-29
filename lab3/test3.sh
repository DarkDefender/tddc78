#!/bin/bash


for n in {1..10}; do
	echo "Iteration: $n" >> "result$2.txt"
	OMP_NUM_THREADS=$1 ./test$2.out >> "result$2.txt"
done

