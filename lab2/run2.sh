#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n ,Cores: $2" >> result1.txt
	echo "Iteration: $n ,Cores: $2" >> result2.txt
	echo "Iteration: $n ,Cores: $2" >> result3.txt
	echo "Iteration: $n ,Cores: $2" >> result4.txt
	$1 ../lab1/images/im1.ppm ./out.ppm $2 >> result1.txt
	$1 ../lab1/images/im2.ppm ./out.ppm $2 >> result2.txt
	$1 ../lab1/images/im3.ppm ./out.ppm $2 >> result3.txt
	$1 ../lab1/images/im4.ppm ./out.ppm $2 >> result4.txt
done

