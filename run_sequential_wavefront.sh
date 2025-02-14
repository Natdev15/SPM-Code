#!/bin/bash
# run_sequential_wavefront.sh
# This script runs the sequential wavefront executable for each matrix size
# a number of times and computes the average elapsed time.
#
# Usage: ./run_sequential_wavefront.sh [MATRIX_SIZE1 MATRIX_SIZE2 ...]
# If no matrix sizes are provided, defaults: 512, 1024, 2048, 4096.

if [ "$#" -eq 0 ]; then
    sizes=(512 1024 2048 4096)
else
    sizes=("$@")
fi

ITER=6
EXE=./sequential_wavefront

for size in "${sizes[@]}"; do
    echo "Running sequential wavefront for matrix size $size..."
    total=0.0
    for (( i=1; i<=ITER; i++ )); do
        output=$($EXE $size)
        # Assume output contains a line: "Elapsed time: X seconds"
        time_val=$(echo "$output" | grep -i "Elapsed time" | awk '{print $3}' | tr -d 's')
        echo "Iteration $i: $time_val s"
        total=$(awk -v a="$total" -v b="$time_val" 'BEGIN {printf "%.6f", a+b}')
    done
    avg=$(awk -v total="$total" -v iter="$ITER" 'BEGIN {printf "%.6f", total/iter}')
    echo "Average time for matrix size $size: $avg s"
    echo ""
done
