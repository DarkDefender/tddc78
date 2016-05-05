#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n ,Cores: 16" >> result1.txt
	echo "Iteration: $n ,Cores: 16" >> result2.txt
	echo "Iteration: $n ,Cores: 16" >> result3.txt
	echo "Iteration: $n ,Cores: 16" >> result4.txt
	mpprun $1 4 4 5000 10000 >> result1.txt
	mpprun $1 4 4 10000 10000 >> result2.txt
	mpprun $1 4 4 50000 10000 >> result3.txt
	mpprun $1 4 4 100000 10000 >> result4.txt
done

