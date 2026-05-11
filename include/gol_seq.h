#ifndef GOL_SEQ_H
#define GOL_SEQ_H

/* 
 * gol_seq.h
 * Core logic for Conway's Game of Life.
 * 
 * current_grid: pointer to the current state of the cells.
 * next_grid: pointer to the allocated memory where the next state will be saved.
 * width, height: dimensions of the grid.
*/

// Computes the next generation of the grid based on the Game of Life rules
void compute_next_generation(const unsigned char* current_grid, unsigned char* next_grid, int width, int height);

#endif