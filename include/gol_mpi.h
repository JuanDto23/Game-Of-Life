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


// Computes the exchange of ghost rows.
void exchange_borders(unsigned char* local_current, int total_width, int local_height,
                      int rank, int num_procs);

// Computes the next generation for the local portion of the grid.
void compute_next_generation(const unsigned char* local_current, unsigned char* local_next, int total_width, int local_height);

// Computes the data gathering of the local grids back into the global grid.
void gather_grid(const unsigned char* local_current, unsigned char* global_grid, int rank, int num_procs, 
                 int total_width, int total_height, int local_height);

#endif