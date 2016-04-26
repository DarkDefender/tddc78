#!/bin/bash

#ifort -openmp ./laplsolv.f90
gfortran -fopenmp -fbounds-check ./laplsolv.f90
