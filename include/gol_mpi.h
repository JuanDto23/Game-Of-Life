#ifndef GOL_MPI_H
#define GOL_MPI_H

#include <mpi.h>

/* * gol_mpi.h
 * Core logic for Conway's Game of Life (parallel version using MPI).
 * 
 * rank: The ID of the calling process.
 * num_procs: Total number of processes.
 * total_height: Total height of the grid.
*/

// Calculates the number of rows that a specific process should compute.
// Returns the local height for the calling process (this does not include ghost cells).
int get_local_height(int rank, int num_procs, int total_height);

// Returns the global row index where the local portion begins.
int get_initial_row(int rank, int num_procs, int total_height);

// Computes the next generation for the local portion of the grid.
//void compute_next_generation(const unsigned char* local_current, unsigned char* local_next, int width, int local_height);

#endif