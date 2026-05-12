#ifndef GOL_MPI_H
#define GOL_MPI_H

#include <mpi.h>

/* * gol_mpi.h
 * Core logic for Conway's Game of Life (parallel version using MPI).
*/

// Computes the initialization of the global grid and domain partitioning among the processes.
// Returns a pointer to the global grid array.
unsigned char* init_parallel_grid(const char* input_file, int rank, int num_procs, 
                                  int* total_width, int* total_height, int* local_height, 
                                  int* initial_row, unsigned char** local_current, unsigned char** local_next);

// Computes the data distribution of the global grid among the local grids of the processes.
void scatter_grid(const unsigned char* global_grid, unsigned char* local_current, int rank, int num_procs, 
                  int total_width, int total_height, int local_height);

// Computes the next generation for the local portion of the grid.
//void compute_next_generation(const unsigned char* local_current, unsigned char* local_next, int width, int local_height);

#endif