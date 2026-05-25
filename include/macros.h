#ifndef MACROS_H_
#define MACROS_H_

/* 
 * macros.h
 * Utilities for memory optimization through bit-packing technique.
 * 
 * array: pointer to one-dimensional array
 * x, y: logical coordinates of the cell on the grid
 * width: total grid width
 * 
 * index_byte = (y * bits_per_row + x) >> 3 (>> 3 = divide by 8)
 * bit_position = x % 8;
*/

// Allows to locate the bit into the byte
#define MASK 0x80 // = (1000 0000) in binary 

// Turn on the bit (it establishes the state of the cell as ALIVE = 1)
#define SET_BIT(array, x, y, bits_per_row)    (array[((y) * (bits_per_row) + (x)) >> 3] |= (MASK >> ((x) % 8)))

// Turn off the bit (it establishes the state of the cell as DEAD = 0)
#define CLEAR_BIT(array, x, y, bits_per_row)  (array[((y) * (bits_per_row) + (x)) >> 3] &= ~(MASK >> ((x) % 8)))

// Extract the value of the bit (return 1 if the cell is ALIVE, 0 if the cell is DEAD)
#define GET_BIT(array, x, y, bits_per_row)    (((array[((y) * (bits_per_row) + (x)) >> 3] << ((x) % 8)) & MASK) ? 1 : 0)

#endif
