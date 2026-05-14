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

  // Main loop GoF.
  for (int gen = 0; gen < generations; gen++) {
    exchange_borders(local_current, total_width, local_height, rank, num_procs);

    compute_next_generation(local_current, local_next, total_width, local_height);

    // Swapping.
    unsigned char* temp = local_current;
    local_current = local_next;
    local_next = temp;
  }

  // Data gathering.
  gather_grid(local_current, global_grid, rank, num_procs, total_width, total_height, local_height);

  // Saving and cleaning.
  if (rank == 0) {
    if (save_pbm(output_file, global_grid, total_width, total_height)) {
      printf("========================================\n");
      printf(" Game of Life - MPI Parallel Execution\n");
      printf("========================================\n");
      printf(" Processes        : %d\n", num_procs);
      printf(" Grid size        : %d x %d\n", total_width, total_height);
      printf(" Generations      : %d\n", generations);
      //printf(" Wall-clock time  : %f seconds\n", time_elapsed);
      printf(" Output saved to  : %s\n", output_file);
      printf("========================================\n");
    }
  }

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