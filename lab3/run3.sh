#!/bin/bash

for i in {1..4}; do
	cores=$( echo "2^$i" | bc )
	echo "Cores: $cores" >> "result$i.txt"
	salloc -N1 -n$cores ./test3.sh $1 $cores $i
done
