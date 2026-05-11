#include <stdio.h>
#include "gol_seq.h"
#include "macros.h"

// Static helper function to count alive neighbors of a cell at (x, y).
// Periodic (toroidal) boundary condition is used.
static int count_alive_neighbors(const unsigned char* grid, int x, int y, int width, int height) 
{
  size_t bytes_per_row = (width + 7) / 8;
  size_t bits_per_row = bytes_per_row * 8;

  int alive_neighbors = 0;

  // Iterate through the Moore neighborhood (3x3 grid around the cell).
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      // Skip the central cell (the cell itself to be analyzed)
      if (dx == 0 && dy == 0) {
        continue;
      }

      // This permits to calculate normal and virtual neighbors
      int neighbor_x = (x + dx + width) % width;
      int neighbor_y = (y + dy + height) % height;

      // Add the state of the neighbor (1 if alive, 0 if dead)
      alive_neighbors += GET_BIT(grid, neighbor_x, neighbor_y, bits_per_row);
    }
  }

  return alive_neighbors;
}

void compute_next_generation(const unsigned char* current_grid, unsigned char* next_grid, int width, int height) 
{
  size_t bytes_per_row = (width + 7) / 8;
  size_t bits_per_row = bytes_per_row * 8;

  // Iterate through every cell in the grid.
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Current state of the cell.
      int is_alive = GET_BIT(current_grid, x, y, bits_per_row);
      
      // Alive neighbors that the current cell has.
      int alive_neighbors = count_alive_neighbors(current_grid, x, y, width, height);

      // Conway's Game of Life rules.
      if (is_alive) {
        // Dies by underpopulation (<= 1) or overpopulation (>= 4)
        if (alive_neighbors <= 1 || alive_neighbors >= 4) {
          CLEAR_BIT(next_grid, x, y, bits_per_row);
        } 
        // Lives on to the next generation (2 or 3)
        else {
          SET_BIT(next_grid, x, y, bits_per_row);
        }
      } else {
        // Reproduction (exactly 3 neighbors)
        if (alive_neighbors == 3) {
          SET_BIT(next_grid, x, y, bits_per_row);
        } 
        // Remains dead
        else {
          CLEAR_BIT(next_grid, x, y, bits_per_row);
        }
      }
    }
  }
}