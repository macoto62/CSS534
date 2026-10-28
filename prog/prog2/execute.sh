#!/bin/bash

#./Wave2D 588 500 0 #>> output.txt
#mpirun -np 1 ./Wave2D_mpi 588 500 0 #>> output_mpi.txt
mpirun -np 1 ./Wave2D_mpi 588 500 0 #>> output_mpi.txt