#include <stdio.h>
#include <stdlib.h>
#include "gol_mpi.h"
#include "macros.h"
#include "pbm_io.h"

// Calculates the number of rows that a specific process should compute.
// Returns the local height for the calling process (this does not include ghost cells).
static int get_local_height(int rank, int num_procs, int total_height) 
{
  // Calculate the base number of rows that every process gets.
  int base_rows = total_height / num_procs;
  
  // Calculate how many rows are leftover.
  int remainder = total_height % num_procs;

  // The leftover rows are distributed among the first processes following the Division Theorem.
  //
  // a = dividend, b = divisor, q = quotient, r = remainder
  //
  // Division Theorem: a = bq + r
  // 0 <= r < abs(b)
  //
  // Based on the above statement, there will be enough rows (r) for distributing among the processes (b). 
  int local_height = base_rows + (rank < remainder ? 1 : 0);
  
  return local_height;
}

// Returns the global row index where the local portion begins.
static int get_initial_row(int rank, int num_procs, int total_height) 
{
  int base_rows = total_height / num_procs;
  int remainder = total_height % num_procs;

  if (rank < remainder) {
    // Processes that received an extra row.
    return rank * (base_rows + 1);
  } else {
    // Processes that received the base number of rows.
    // They must shift their initial index to account for the extra rows given to the first 'remainder' processes.
    return rank * base_rows + remainder;
  }
}

unsigned char* init_parallel_grid(const char* input_file, int rank, int num_procs, 
                                  int* total_width, int* total_height, int* local_height, 
                                  int* initial_row, unsigned char** local_current, unsigned char** local_next)
{
  // Only the rank 0 process will use this.
  unsigned char* global_grid = NULL;

  // The master node (rank 0) will read the input PBM file.
  if (rank == 0) {
    global_grid = read_pbm(input_file, total_width, total_height);
    if (!global_grid) {
      // If it fails, terminates MPI execution environment.
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // The master node (0) broadcast the dimensions to the rest of the slave nodes.
  MPI_Bcast(total_width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(total_height, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Local partitioning calculations.
  *local_height = get_local_height(rank, num_procs, *total_height);
  *initial_row = get_initial_row(rank, num_procs, *total_height);

  // To each process is added 2 extra rows (one up, one down) for saving the cells of the neighbors. These rows are named "ghost cells".
  int local_height_with_ghosts = *local_height + 2;
  size_t bytes_per_row = (*total_width + 7) / 8;
  size_t local_bytes = bytes_per_row * local_height_with_ghosts;

  // Memory allocation for local and next grid of each process.
  *local_current = (unsigned char*) calloc(local_bytes, sizeof(unsigned char));
  *local_next = (unsigned char*) calloc(local_bytes, sizeof(unsigned char));

  if (!(*local_current) || !(*local_next)) {
    fprintf(stderr, "Error: Memory allocation failed on rank %d.\n", rank);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  return global_grid;
}

void scatter_grid(const unsigned char* global_grid, unsigned char* local_current, int rank, int num_procs, 
                  int total_width, int total_height, int local_height)
{
  size_t bytes_per_row = (total_width + 7) / 8;
  int* sendcounts = NULL;
  int* displs = NULL;

  if (rank == 0) {
    sendcounts = (int*) malloc(num_procs * sizeof(int));
    displs = (int*) malloc(num_procs * sizeof(int));

    for (int i = 0; i < num_procs; i++) {
      // Number of rows of 'i' process.
      int h = get_local_height(i, num_procs, total_height);
      // Initial row of 'i' process.
      int initial = get_initial_row(i, num_procs, total_height);

      // This receives the number of bytes that corresponds to the 'i' process.
      sendcounts[i] = bytes_per_row * h;
      // This receives the index of byte where the 'i' process begins.
      displs[i] = bytes_per_row * initial;
    }
  }

  // Calculates how many bytes this specific process really waits to receive (not counting the ghost cells).
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
}