#!/bin/bash

#Test1

tar -xf ../lab1/mpi/blur_result_1.tar
./split_lab1_data.sh result4.txt
python plot1.py blur1

rm result*
rm number*

tar -xf ../lab1/mpi/thres_result_1.tar
./split_lab1_data.sh result4.txt
python plot1.py thres1

rm result*
rm number*

#Test2

tar -xf ../lab1/mpi/blur_result_2.tar

./split_lab1_data.sh result1.txt
cat number* > prob01

./split_lab1_data.sh result2.txt
cat number* > prob02

./split_lab1_data.sh result3.txt
cat number* > prob03

./split_lab1_data.sh result4.txt
cat number* > prob04

python plot2.py blur2

rm result*
rm number*
rm prob*

tar -xf ../lab1/mpi/thres_result_2.tar

./split_lab1_data.sh result1.txt
cat number* > prob01

./split_lab1_data.sh result2.txt
cat number* > prob02

./split_lab1_data.sh result3.txt
cat number* > prob03

./split_lab1_data.sh result4.txt
cat number* > prob04

python plot2.py thres2

rm result*
rm number*
rm prob*

#Test3

tar -xf ../lab1/mpi/blur_result_3.tar

./split_lab1_data.sh result1.txt
cat number* > prob01

./split_lab1_data.sh result2.txt
cat number* > prob02

./split_lab1_data.sh result3.txt
cat number* > prob03

./split_lab1_data.sh result4.txt
cat number* > prob04

python plot2.py blur3

rm result*
rm number*
rm prob*

tar -xf ../lab1/mpi/thres_result_3.tar

./split_lab1_data.sh result1.txt
cat number* > prob01

./split_lab1_data.sh result2.txt
cat number* > prob02

./split_lab1_data.sh result3.txt
cat number* > prob03

./split_lab1_data.sh result4.txt
cat number* > prob04

python plot2.py thres3

rm result*
rm number*
rm prob*
