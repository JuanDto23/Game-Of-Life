#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "pbm_io.h"
#include "macros.h"

unsigned char* read_pbm(const char* file_name, int* width, int* height)
{
  FILE* f = fopen(file_name, "r");
  if (!f) {
    fprintf(stderr, "Error: The file %s could not be opened for reading.\n", file_name);
    return NULL;
  }

  char magic[3];
  fscanf(f, "%2s", magic);
  if (magic[0] != 'P' || magic[1] != '1') {
    fprintf(stderr, "Error: The file %s is not a valid ASCII (P1) PBM.\n", file_name);
    fclose(f);
    return NULL;
  }

  fscanf(f, "%d %d", width, height);

  // 7 bits (or cells) are added in total_bytes guaranteeing to reserve an aditional byte in those cases where the grid dimensions are not a multiple of 8.
  size_t total_cells = (*width) * (*height);
  size_t total_bytes = (total_cells + 7) / 8;
  
  unsigned char* grid = (unsigned char*) calloc(total_bytes, sizeof(unsigned char));
  if (!grid) {
    fprintf(stderr, "Error: Memory allocation failed.\n");
    fclose(f);
    return NULL;
  }

  int pixel;
  for (int y = 0; y < *height; y++) {
    for (int x = 0; x < *width; x++) {
      fscanf(f, "%d", &pixel);
      if (pixel == 1) {
        SET_BIT(grid, x, y, *width);
      }
    }
  }

  fclose(f);
  return grid;
}

int save_pbm(const char* file_name, const unsigned char* grid, int width, int height)
{
  FILE* f = fopen(file_name, "w");
  if (!f) {
    fprintf(stderr, "Error: The file %s could not be created.\n", file_name);
    return 0;
  }

  fprintf(f, "P1\n");
  fprintf(f, "%d %d\n", width, height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int bit = GET_BIT(grid, x, y, width);
      fprintf(f, "%d ", bit);
    }
    // A new line is introduced to make it easier to read.
    fprintf(f, "\n");
  }

  fclose(f);
  return 1;
}