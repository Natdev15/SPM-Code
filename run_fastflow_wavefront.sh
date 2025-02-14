#!/bin/bash
# run_fastflow_wavefront.sh
# This script runs the FastFlow farm version for each specified matrix size.
# Usage: ./run_fastflow_wavefront.sh [MATRIX_SIZE1 MATRIX_SIZE2 ...] [--threads NUM]
# If no matrix sizes are provided, defaults: 512, 1024, 2048, 4096.
# If --threads is not provided, default number of threads is 32.

threads=32
args=()
while [[ $# -gt 0 ]]; do
    case $1 in
        --threads)
            threads=$2
            shift 2;;
        *)
            args+=("$1")
            shift;;
    esac
done

if [ ${#args[@]} -eq 0 ]; then
    sizes=(512 1024 2048 4096)
else
    sizes=("${args[@]}")
fi

EXE=./fastflow_wavefront_farm

for size in "${sizes[@]}"; do
    echo "Running FastFlow wavefront for matrix size $size with $threads threads..."
    $EXE $size $threads
    echo ""
done
