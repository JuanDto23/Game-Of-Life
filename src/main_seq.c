#include <stdio.h>
#include <stdlib.h>
#include "macros.h"
#include "pbm_io.h"
#include "gol_seq.h"

int main()
{
  // A test
  int width, height;
  unsigned char* grid = read_pbm("data/input.pbm", &width, &height);
  unsigned char* next_grid = (unsigned char*) calloc((width * height + 7) / 8, sizeof(unsigned char));

  for (int g = 0; g < 3; g++) {
    compute_next_generation(grid, next_grid, width, height);
    free(grid);
    grid = next_grid;
    next_grid = (unsigned char*) calloc((width * height + 7) / 8, sizeof(unsigned char));
  }

  save_pbm("data/output.pbm", grid, width, height);
}