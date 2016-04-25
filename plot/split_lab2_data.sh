#!/bin/bash

sed -i '1d' $1

csplit -f number $1 -z '/Cores/+1' '{*}'

sed -i '/secs/!d' number*
sed -i 's/[^0-9\.]*//g' number*
