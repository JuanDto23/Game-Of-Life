#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "pbm_io.h"
#include "gol_seq.h"

int main(int argc, char* argv[]) 
{
  // Input arguments validation.
  if (argc != 4) {
    printf("Usage: %s <input.pbm> <output.pbm> <generations>\n", argv[0]);
    return 1;
  }

  const char* input_file = argv[1];
  const char* output_file = argv[2];
  int generations = atoi(argv[3]);

  if (generations <= 0) {
    printf("Error: The number of generations must be greater than 0.\n");
    return 1;
  }

  // Reading the initial state of the grid.
  int width, height;
  unsigned char* current_grid = read_pbm(input_file, &width, &height);
  if (!current_grid) {
    return 1;
  }

  // Memory allocation for the next state of the grid.
  size_t bytes_per_row = (width + 7) / 8;
  size_t total_bytes = bytes_per_row * height;
  unsigned char* next_grid = (unsigned char*) calloc(total_bytes, sizeof(unsigned char));
  if (!next_grid) {
    fprintf(stderr, "Error: Memory allocation for next_grid failed.\n");
    free(current_grid);
    return 1;
  }

  // Chronometer configuration.
  struct timeval start, finish;
  
  gettimeofday(&start, NULL);

  // Main loop of the simulation.
  for (int i = 0; i < generations; i++) {
    compute_next_generation(current_grid, next_grid, width, height);

    // Swapping
    unsigned char* temp = current_grid;
    current_grid = next_grid;
    next_grid = temp;
  }
  
  gettimeofday(&finish, NULL);

  // Wall-clock time
  double time_elapsed = (finish.tv_sec - start.tv_sec) + 
                        (finish.tv_usec - start.tv_usec) / 1000000.0;

  // Metrics.
  if (save_pbm(output_file, current_grid, width, height)) {
    printf("========================================\n");
    printf(" Game of Life - Sequential Execution\n");
    printf("========================================\n");
    printf(" Grid size        : %d x %d\n", width, height);
    printf(" Generations      : %d\n", generations);
    printf(" Wall-clock time  : %f seconds\n", time_elapsed);
    printf(" Output saved to  : %s\n", output_file);
    printf("========================================\n");
  }

  free(current_grid);
  free(next_grid);

  return 0;
}