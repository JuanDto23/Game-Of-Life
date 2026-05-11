#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>
#include "pbm_io.h"
#include "gol_mpi.h"
#include "macros.h" //DEBUG

int main(int argc, char* argv[])
{
  // Start up MPI
  MPI_Init(&argc, &argv);

  int rank, num_procs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

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
  
  int total_width = 0;
  int total_height = 0;
  // Only the rank 0 process will use this.
  unsigned char* global_grid = NULL;

  // The master node (rank 0) will read the input PBM file.
  if (rank == 0) {
    global_grid = read_pbm(input_file, &total_width, &total_height);
    if (!global_grid) {
      // If it fails, terminates MPI execution environment.
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // The master node (0) broadcast the dimensions to the rest of the slave nodes.
  MPI_Bcast(&total_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&total_height, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Local partitioning calculations.
  int local_height = get_local_height(rank, num_procs, total_height);
  int initial_row = get_initial_row(rank, num_procs, total_height);

  // To each process is added 2 extra rows (one up, one down) for saving the cells of the neighbors. These rows are named "ghost cells".
  int local_height_with_ghosts = local_height + 2;
  size_t bytes_per_row = (total_width + 7) / 8;
  size_t local_bytes = bytes_per_row * local_height_with_ghosts;

  // Memory allocation for local and next grid of each process.
  unsigned char* local_current = (unsigned char*) calloc(local_bytes, sizeof(unsigned char));
  unsigned char* local_next = (unsigned char*) calloc(local_bytes, sizeof(unsigned char));

  if (!local_current || !local_next) {
    fprintf(stderr, "Error: Memory allocation failed on rank %d.\n", rank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  // Data distribution of the global grid among processes.
  int* sendcounts = NULL;
  int* displs = NULL;

  if (rank == 0) {
    sendcounts = (int*) malloc(num_procs * sizeof(int));
    displs = (int*) malloc(num_procs * sizeof(int));

    for (int i = 0; i < num_procs; i++) {
      // Rows of 'i' process.
      int h = get_local_height(i, num_procs, total_height);
      // Initial row of 'i' process.
      int initial = get_initial_row(i, num_procs, total_height);

      // This receives the number of bytes that corresponds to the 'i' process.
      sendcounts[i] = bytes_per_row * h;
      // This receives the index of byte where the 'i' process begins.
      displs[i] = bytes_per_row * initial;
    }
  }

  // Calculates how many bytes this specific process really waits to receive (not counting the ghost cells yet).
  int local_recv_count = bytes_per_row * local_height;

  // The data is not saved at the beginning because the first row is the upper ghost row. The destination pointer is displaced to the next row.
  unsigned char* recv_buffer_ptr = local_current + bytes_per_row;

  MPI_Scatterv(global_grid, sendcounts, displs, MPI_UNSIGNED_CHAR,
                recv_buffer_ptr, local_recv_count, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

  // Set free the auxiliary arrays of master node (0).
  if (rank == 0) {
    free(sendcounts);
    free(displs);
  }

  // Main loop GoF

  free(local_current);
  free(local_next);
  if (rank == 0 && global_grid != NULL) {
    free(global_grid);
  }

  // Finalization of MPI.
  MPI_Finalize();
  return 0;
}