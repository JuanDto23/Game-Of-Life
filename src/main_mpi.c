#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "pbm_io.h"
#include "gol_mpi.h"

int main(int argc, char* argv[])
{
  // Start up MPI.
  MPI_Init(&argc, &argv);

  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  // Parameters validation.
  if (argc != 4) {
    if (rank == 0) {
      printf("Usage: %s <input.pbm> <output.pbm> <generations>\n", argv[0]);
    }
    MPI_Finalize();
    return 1;
  }

  const char* input_file = argv[1];
  const char* output_file = argv[2];
  int generations = atoi(argv[3]);

  if (generations <= 0) {
    if (rank == 0) {
      printf("Error: The number of generations must be greater than 0.\n");
    }
    MPI_Finalize();
    return 1;
  }
  
  // Memory allocation for global grid and domain partitioning.
  int total_width = 0;
  int total_height = 0;
  int local_height = 0;
  int initial_row = 0;
  unsigned char* local_current = NULL;
  unsigned char* local_next = NULL;

  unsigned char* global_grid = init_parallel_grid(input_file, rank, num_procs, 
                                                  &total_width, &total_height, &local_height, 
                                                  &initial_row, &local_current, &local_next);

  // Data distribution of the global grid among processes.
  scatter_grid(global_grid, local_current, rank, num_procs, 
               total_width, total_height, local_height);

  // Main loop GoF

  // Memory deallocation.
  free(local_current);
  free(local_next);
  if (rank == 0 && global_grid != NULL) {
    free(global_grid);
  }

  // Finalization of MPI.
  MPI_Finalize();
  return 0;
}