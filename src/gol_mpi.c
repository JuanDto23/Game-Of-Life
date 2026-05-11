#include "gol_mpi.h"
#include "macros.h"

int get_local_height(int rank, int num_procs, int total_height) 
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

int get_initial_row(int rank, int num_procs, int total_height) 
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