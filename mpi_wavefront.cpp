#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <mpi.h>
using namespace std;

void init_matrix(double *M, uint64_t N) {
    for (uint64_t i = 0; i < N; i++)
        for (uint64_t j = 0; j < N; j++)
            M[i * N + j] = (j == i) ? (i + 1) / static_cast<double>(N) : 0.0;
}

void wavefront_MPI(double *M, uint64_t N, int myRank, int size) {
    for (uint64_t k = 1; k < N; ++k) {
        uint64_t tasks = N - k;
        int base = tasks / size, rem = tasks % size;
        int local_count = base + (myRank < rem ? 1 : 0);
        int local_start = myRank * base + std::min(myRank, rem);
        
        vector<double> local_vals(local_count, 0.0);
        vector<int> local_idx(local_count, 0);
        for (int i = 0; i < local_count; ++i) {
            uint64_t idx = local_start + i;
            double dot = 0.0;
            for (uint64_t j = 1; j <= k; ++j)
                dot += M[idx * N + (idx + k - j)] * M[(idx + j) * N + (idx + k)];
            local_vals[i] = cbrt(dot);
            local_idx[i] = idx;
        }
        
        int local_count_int = local_count;
        vector<int> recv_counts(size, 0);
        MPI_Allgather(&local_count_int, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);
        
        vector<int> displs(size, 0);
        int total = 0;
        for (int r = 0; r < size; r++) {
            displs[r] = total;
            total += recv_counts[r];
        }
        
        vector<double> gathered_vals(total, 0.0);
        vector<int> gathered_idx(total, 0);
        MPI_Allgatherv(local_vals.data(), local_count_int, MPI_DOUBLE,
                       gathered_vals.data(), recv_counts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);
        MPI_Allgatherv(local_idx.data(), local_count_int, MPI_INT,
                       gathered_idx.data(), recv_counts.data(), displs.data(), MPI_INT, MPI_COMM_WORLD);
        
        for (int i = 0; i < total; i++) {
            int idx = gathered_idx[i];
            M[idx * N + (idx + k)] = gathered_vals[i];
        }
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    int myRank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    uint64_t N = 512;  // default matrix size
    if (myRank == 0 && argc > 1) {
        try {
            N = stoull(argv[1]);
        } catch (...) {
            cerr << "Invalid matrix size." << endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    }
    MPI_Bcast(&N, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    double *M = new(nothrow) double[N * N];
    if (!M) {
        cerr << "Memory allocation failed." << endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    init_matrix(M, N);

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();
    wavefront_MPI(M, N, myRank, size);
    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed = MPI_Wtime() - start_time;

    double checksum = 0.0;
    for (uint64_t i = 0; i < N * N; i++) checksum += M[i];

    if (myRank == 0) {
        cout << "# Matrix size: " << N << "x" << N << endl;
        cout << "# Number of processes: " << size << endl;
        cout << "# Elapsed time (wavefront): " << elapsed << " seconds" << endl;
        cout << "# Final matrix checksum: " << checksum << endl;
    }

    delete[] M;
    MPI_Finalize();
    return 0;
}
