#!/bin/bash

#Test1

tar -xf ../lab3/fort1.tar
./split_lab3_data.sh result4.txt
python plot1.py fort1

rm result*
rm number*

#Test2

tar -xf ../lab3/fort2.tar

./split_lab3_data.sh result1.txt
cat number* > prob01

./split_lab3_data.sh result2.txt
cat number* > prob02

./split_lab3_data.sh result3.txt
cat number* > prob03

./split_lab3_data.sh result4.txt
cat number* > prob04

python plot2.py fort2

rm result*
rm number*
rm prob*

#Test3
tar -xf ../lab3/fort3.tar

./split_lab3_data.sh result1.txt
cat number* > prob01

./split_lab3_data.sh result2.txt
cat number* > prob02

./split_lab3_data.sh result3.txt
cat number* > prob03

./split_lab3_data.sh result4.txt
cat number* > prob04

python plot2.py fort3

rm result*
rm number*
rm prob*
