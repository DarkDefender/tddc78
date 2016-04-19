#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n ,Cores: $2" >> result1.txt
	echo "Iteration: $n ,Cores: $2" >> result2.txt
	echo "Iteration: $n ,Cores: $2" >> result3.txt
	echo "Iteration: $n ,Cores: $2" >> result4.txt
	mpprun $1 ../images/im1.ppm >> result1.txt
	mpprun $1 ../images/im2.ppm >> result2.txt
	mpprun $1 ../images/im3.ppm >> result3.txt
	mpprun $1 ../images/im4.ppm >> result4.txt
done

