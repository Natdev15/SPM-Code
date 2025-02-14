#!/bin/bash
# run_mpi_wavefront.sh
# This script runs the MPI wavefront executable using srun.
# Usage: ./run_mpi_wavefront.sh MATRIX_SIZE NODES TASKS
# Example: ./run_mpi_wavefront.sh 1024 8 256
# (In this example, 8 nodes with a total of 256 MPI processes.)

if [ "$#" -ne 3 ]; then
    echo "Usage: $0 MATRIX_SIZE NODES TASKS"
    exit 1
fi

MATRIX_SIZE=$1
NODES=$2
TASKS=$3
EXE=./mpi_wavefront

srun -N $NODES -n $TASKS $EXE $MATRIX_SIZE
