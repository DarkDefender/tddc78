#!/bin/bash


for n in {1..10}; do
	echo "Iteration: $n" >> "result$2.txt"
	mpprun $1 "../images/im$2.ppm" >> "result$2.txt"
done

