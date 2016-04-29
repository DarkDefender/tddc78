#!/bin/bash

for n in {1..10}; do
	echo "Iteration: $n" >> $3
	$1 $2 >> $3
done

