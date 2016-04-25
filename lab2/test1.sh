#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n" >> $3
	$1 ../lab1/images/im4.ppm ./out.ppm $2 >> $3
done

