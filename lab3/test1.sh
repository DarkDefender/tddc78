#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n" >> $3
	OMP_NUM_THREADS=$2 $1 >> $3
done

