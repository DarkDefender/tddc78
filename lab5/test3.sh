#!/bin/bash


for n in {1..10}; do
	echo "Iteration: $n" >> $6
	mpprun $1 $2 $3 $4 $5  >> $6
done

