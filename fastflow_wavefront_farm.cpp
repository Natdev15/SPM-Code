#include <iostream>
#include <cmath>
#include <vector>
#include <ff/farm.hpp>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "hpc_helpers.hpp"

using namespace std;
using namespace ff;

// Task structure: compute one element on the k-th diagonal.
struct Task {
    uint64_t i, k;
};

// Emitter: produces tasks for a given diagonal offset k.
struct Emitter : public ff_node {
    uint64_t N, k, cur, tasks;
    Emitter(uint64_t N, uint64_t k) : N(N), k(k), cur(0), tasks(N - k) { }
    void* svc(void*) override { 
        return (cur < tasks ? new Task{cur++, k} : nullptr); 
    }
};

// Worker: processes a task by computing the dot product and updating M.
struct Worker : public ff_node {
    double* M;
    uint64_t N;
    Worker(double* M, uint64_t N) : M(M), N(N) { }
    void* svc(void* t) override {
        Task* task = static_cast<Task*>(t);
        uint64_t i = task->i, k = task->k;
        double dot = 0.0;
        for (uint64_t j = 1; j <= k; ++j)
            dot += M[i * N + (i + k - j)] * M[(i + j) * N + (i + k)];
        M[i * N + (i + k)] = cbrt(dot);
        delete task;
        return GO_ON;
    }
};

// Collector: dummy node.
struct Collector : public ff_node {
    void* svc(void* t) override { return GO_ON; }
};

// Initializes an N x N matrix: diagonal elements are (i+1)/N, off-diagonals 0.
void init_matrix(double* M, uint64_t N) {
    for (uint64_t i = 0; i < N; i++)
        for (uint64_t j = 0; j < N; j++)
            M[i * N + j] = (j == i ? (i + 1) / static_cast<double>(N) : 0.0);
}

// Prints a simple summary: checksum of M.
void print_summary(double* M, uint64_t N) {
    double checksum = 0.0;
    for (uint64_t i = 0; i < N * N; i++) checksum += M[i];
    cout << "Checksum: " << checksum << endl;
}

int main(int argc, char* argv[]) {
    uint64_t N = 512;       // Default matrix size.
    int numWorkers = 32;    // Default number of worker threads.
    if (argc >= 2) N = stoull(argv[1]);
    if (argc >= 3) numWorkers = stoi(argv[2]);

    double* M = new double[N * N];
    init_matrix(M, N);

    TIMERSTART(total);
    // For each diagonal offset k, create a farm to process that diagonal.
    for (uint64_t k = 1; k < N; k++) {
        Emitter emitter(N, k);
        Collector collector;
        vector<ff_node*> workers;
        for (int i = 0; i < numWorkers; i++) {
            workers.push_back(new Worker(M, N));
        }
        ff_Farm farm(move(workers), &emitter, &collector);
        if (farm.run_then_freeze() < 0) {
            cerr << "Error running farm" << endl;
            return -1;
        }
        farm.wait();
    }
    TIMERSTOP(total);

    cout << "# Elapsed time (farm, 32 threads, no affinity): " << ffTime(GET_TIME)/1000 << "s" << endl;
    print_summary(M, N);

    delete[] M;
    return 0;
}
