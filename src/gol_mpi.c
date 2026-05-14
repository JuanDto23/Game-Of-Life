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

void exchange_borders(unsigned char* local_current, int total_width, int local_height,
                      int rank, int num_procs)
{    
  size_t bytes_per_row = (total_width + 7) / 8;
  // Neighbors identification.
  // Uses modulo arithmetic to create a logical ring of processes (toroidal topology).
  int neighbor_up = (rank - 1 + num_procs) % num_procs;
  int neighbor_down = (rank + 1) % num_procs;

  // Setup pointers for the specific boundaries of the local grid.
  // The upper ghost row is at index 0.
  unsigned char* ghost_top = local_current;
  // The first real row is at index 1.
  unsigned char* first_real_row = local_current + bytes_per_row;
  // The last real row is at index 'local_height'.
  unsigned char* last_real_row = local_current + (local_height * bytes_per_row);
  // The lower ghost row is at index 'local_height + 1'.
  unsigned char* ghost_bottom = local_current + ((local_height + 1) * bytes_per_row);

  // Communication UP / Receive from DOWN.
  // Sends the first real row to the upper neighbor.
  // Receives the first real row from the lower neighbor into the bottom ghost cell.
  MPI_Sendrecv(first_real_row, bytes_per_row, MPI_UNSIGNED_CHAR, neighbor_up, 0,
               ghost_bottom, bytes_per_row, MPI_UNSIGNED_CHAR, neighbor_down, 0,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  // Communication DOWN / Receive from UP.
  // Sends the last real row to the lower neighbor.
  // Receives the last real row from the upper neighbor into the top ghost cell.
  MPI_Sendrecv(last_real_row, bytes_per_row, MPI_UNSIGNED_CHAR, neighbor_down, 1,
               ghost_top, bytes_per_row, MPI_UNSIGNED_CHAR, neighbor_up, 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

// Static helper function to count alive neighbors of a cell at (x, y) in the local grid.
// Periodic (toroidal) boundary condition is used only for the x-axis.
// The y-axis relies on the exchanged ghost cells.
static int count_alive_neighbors(const unsigned char* grid, int x, int y, int total_width) 
{
  size_t bytes_per_row = (total_width + 7) / 8;
  size_t bits_per_row = bytes_per_row * 8;

  int alive_neighbors = 0;

  // Iterate through the Moore neighborhood (3x3 grid around the cell).
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      // Skip the central cell.
      if (dx == 0 && dy == 0) {
        continue;
      }

      // X-axis remains toroidal.
      int neighbor_x = (x + dx + total_width) % total_width;
      
      // Y-axis no longer uses modulo. The boundary cell are the ghost cells.
      int neighbor_y = y + dy;

      // Add the state of the neighbor.
      alive_neighbors += GET_BIT(grid, neighbor_x, neighbor_y, bits_per_row);
    }
  }

  return alive_neighbors;
}

void compute_next_generation(const unsigned char* local_current, unsigned char* local_next, int total_width, int local_height)
{
  size_t bytes_per_row = (total_width + 7) / 8;
  size_t bits_per_row = bytes_per_row * 8;

  // Iterates only through the real cells assigned to this process.
  // Row 0 is the top ghost row. Row 'local_height + 1' is the bottom ghost row.
  for (int y = 1; y <= local_height; y++) {
    for (int x = 0; x < total_width; x++) {
      int is_alive = GET_BIT(local_current, x, y, bits_per_row);
      int alive_neighbors = count_alive_neighbors(local_current, x, y, total_width);

      // Conway's Game of Life rules.
      if (is_alive) {
        if (alive_neighbors <= 1 || alive_neighbors >= 4) {
          CLEAR_BIT(local_next, x, y, bits_per_row);
        } else {
          SET_BIT(local_next, x, y, bits_per_row);
        }
      } else {
        if (alive_neighbors == 3) {
          SET_BIT(local_next, x, y, bits_per_row);
        } else {
          CLEAR_BIT(local_next, x, y, bits_per_row);
        }
      }
    }
  }
}

void gather_grid(const unsigned char* local_current, unsigned char* global_grid, int rank, int num_procs, 
                 int total_width, int total_height, int local_height)
{
  size_t bytes_per_row = (total_width + 7) / 8;
  int* recvcounts = NULL;
  int* displs = NULL;

  if (rank == 0) {
    recvcounts = (int*) malloc(num_procs * sizeof(int));
    displs = (int*) malloc(num_procs * sizeof(int));

    for (int i = 0; i < num_procs; i++) {
      int h = get_local_height(i, num_procs, total_height);
      int initial = get_initial_row(i, num_procs, total_height);

      recvcounts[i] = bytes_per_row * h;
      displs[i] = bytes_per_row * initial;
    }
  }

  // Calculate the exact amount of real data to send (ignoring ghost cells).
  int local_send_count = bytes_per_row * local_height;

  // Shift the pointer by 1 row to skip the top ghost cell.
  const unsigned char* send_buffer_ptr = local_current + bytes_per_row;

  MPI_Gatherv(send_buffer_ptr, local_send_count, MPI_UNSIGNED_CHAR,
              global_grid, recvcounts, displs, MPI_UNSIGNED_CHAR,
              0, MPI_COMM_WORLD);

  // Clean up.
  if (rank == 0) {
    free(recvcounts);
    free(displs);
  }
}