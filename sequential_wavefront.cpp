#include <iostream>
#include <cmath>
#include <vector>
#include "hpc_helpers.hpp"  

using namespace std;

/**
 * Initializes the matrix M of size N x N.
 * Sets the main diagonal to (i+1)/N and all off-diagonal elements to 0.0.
 */
void init_matrix(double *M, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (j == i)
                M[i * N + j] = (i + 1) / static_cast<double>(N);
            else
                M[i * N + j] = 0.0;
        }
    }
}

/**
 * Prints the entire matrix M of size N x N.
 * (Intended for small matrices only.)
 */
void print_matrix(double *M, uint64_t N) {
    printf("Resulting Matrix:\n");
    for (uint64_t i = 0; i < N; ++i) {
        for (uint64_t j = 0; j < N; ++j) {
            printf("%f ", M[i * N + j]);
        }
        printf("\n");
    }
}

/**
 * Prints a summary of the matrix M of size N x N.
 * For large matrices, prints the checksum (sum of all elements) and
 * the top-left 10x10 block.
 */
void print_matrix_summary(double *M, uint64_t N) {
    double checksum = 0.0;
    for (uint64_t i = 0; i < N * N; i++) {
        checksum += M[i];
    }
    cout << "Matrix checksum: " << checksum << endl;
    cout << "Top-left 10x10 block:" << endl;
    uint64_t limit = (N < 10) ? N : 10;
    for (uint64_t i = 0; i < limit; ++i) {
        for (uint64_t j = 0; j < limit; ++j) {
            cout << M[i * N + j] << "\t";
        }
        cout << endl;
    }
}

/**
 * Performs the wavefront computation on matrix M of size N x N.
 * For each diagonal offset k (from 1 to N-1), computes the element M[i][i+k]
 * as the cube root of the dot product over k elements.
 */
void wavefront(double *M, uint64_t N) {
    for (uint64_t k = 1; k < N; ++k) {
        for (uint64_t i = 0; i < N - k; ++i) {
            double dotProduct = 0.0;
            // Compute dot product over k elements.
            for (uint64_t j = 1; j < k + 1; ++j) {
                dotProduct += M[i * N + (i + k - j)] * M[(i + j) * N + (i + k)];
            }
            // Update the matrix element using the cube root.
            M[i * N + (i + k)] = cbrt(dotProduct);
        }
    }
}

int main(int argc, char *argv[]) {
    // Default matrix size is 512.
    uint64_t N = 512;
    if (argc != 1 && argc != 2) {
        cerr << "Usage: " << argv[0] << " N" << endl;
        cerr << "       N: size of the square matrix" << endl;
        return -1;
    }
    if (argc > 1) {
        try {
            N = std::stoull(argv[1]);
        } catch (...) {
            cerr << "Invalid matrix size." << endl;
            return -1;
        }
    }
    
    // Allocate memory for the matrix.
    double *M = new(nothrow) double[N * N];
    if (!M) {
        cerr << "Memory allocation failed for a " << N << "x" << N << " matrix." << endl;
        return -1;
    }
    
    // Initialize the matrix.
    init_matrix(M, N);
    
    // Start the timer for the wavefront computation.
    TIMERSTART(wavefront);
    wavefront(M, N); 
    TIMERSTOP(wavefront);
    
    // Print a summary of the matrix.
    if (N <= 20)
        print_matrix(M, N);
    else
        print_matrix_summary(M, N);
    
    // Optionally, you could print one element (e.g., the last element) for quick checking:
    // cout << "M[N-1] = " << M[N-1] << endl;
    
    delete[] M;
    return 0;
}
