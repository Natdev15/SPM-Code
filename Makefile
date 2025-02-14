# Makefile for the Wavefront Computation Project

# Compilers:
CXX      = g++
MPICXX   = mpicxx

# Compiler Flags:
CXXFLAGS = -O2 -std=c++11
LDFLAGS  = -pthread

# FastFlow library flag (adjust as needed)
FFLIB    = -lff

# Executable names:
SEQ_EXEC      = sequential_wavefront_with_helpers
FASTFLOW_EXEC = fastflow_farm_no_affinity
MPI_EXEC      = mpi_wavefront_improved_timed

.PHONY: all sequential fastflow mpi clean

all: sequential fastflow mpi

# Sequential version (uses hpc_helpers.hpp for timing)
sequential: $(SEQ_EXEC)

$(SEQ_EXEC): sequential_wavefront_with_helpers.cpp hpc_helpers.hpp
	$(CXX) $(CXXFLAGS) sequential_wavefront_with_helpers.cpp -o $(SEQ_EXEC) $(LDFLAGS)

# FastFlow version (farm-based, no CPU affinity)
fastflow: $(FASTFLOW_EXEC)

$(FASTFLOW_EXEC): fastflow_farm_no_affinity.cpp hpc_helpers.hpp
	$(CXX) $(CXXFLAGS) fastflow_farm_no_affinity.cpp -o $(FASTFLOW_EXEC) $(LDFLAGS) $(FFLIB)

# MPI version
mpi: $(MPI_EXEC)

$(MPI_EXEC): mpi_wavefront_improved_timed.cpp hpc_helpers.hpp
	$(MPICXX) $(CXXFLAGS) mpi_wavefront_improved_timed.cpp -o $(MPI_EXEC)

clean:
	rm -f $(SEQ_EXEC) $(FASTFLOW_EXEC) $(MPI_EXEC)
