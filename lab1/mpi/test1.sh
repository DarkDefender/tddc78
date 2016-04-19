#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n" >> $2
	mpprun $1 ../images/im4.ppm >> $2
done

