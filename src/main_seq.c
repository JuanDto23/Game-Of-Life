#include <stdio.h>
#include "macros.h"
#include "pbm_io.h"

int main()
{
  // A test
  int width, height;
  unsigned char* grid = read_pbm("data/input.pbm", &width, &height);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int bit = GET_BIT(grid, x, y, width);
      printf("%d ", bit);
    }
    printf("\n");
  }
}