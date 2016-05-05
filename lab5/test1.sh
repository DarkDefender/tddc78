#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n" >> $2
	mpprun $1 $2 $3 10000 10000 >> $4
done

