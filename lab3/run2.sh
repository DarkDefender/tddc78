#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n ,Cores: $1" >> result1.txt
	echo "Iteration: $n ,Cores: $1" >> result2.txt
	echo "Iteration: $n ,Cores: $1" >> result3.txt
	echo "Iteration: $n ,Cores: $1" >> result4.txt
	./test1.out $1 >> result1.txt
	./test2.out $1 >> result2.txt
	./test3.out $1 >> result3.txt
	./test4.out $1 >> result4.txt
done

