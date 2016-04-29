#!/bin/bash


for n in {1..10}; do
	echo "Iteration: $n" >> "result$3.txt"
	$1 "../lab1/images/im$3.ppm" ./out.ppm $2 >> "result$3.txt"
done

