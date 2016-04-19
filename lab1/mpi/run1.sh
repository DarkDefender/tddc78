#!/bin/bash

for i in {0..4}; do
	cores=$( echo "2^$i" | bc )
	echo "Cores: $cores" >> result4.txt
	salloc -N1 -n$cores ./test1.sh $1 result4.txt
done
