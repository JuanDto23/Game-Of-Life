#ifndef PBM_IO_H
#define PBM_IO_H

/* 
 * pbm_io.h
 * Functions to read/write the state of the grid in PBM format.
*/

// It reads a PBM file and returns a pointer to the bit array (grid).
// It modifies the pointers of width and height with the dimensions read of the PBM file. 
unsigned char* read_pbm(const char* file_name, int* width, int* height);

// It saves the bit array (grid) in a PBM file.
// It returns 1 if successful, 0 if there was an error.
int save_pbm(const char* file_name, const unsigned char* grid, int width, int height);

#endif