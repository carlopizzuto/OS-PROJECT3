#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Utility functions used throughout the project
 */

/**
 * Print an error message and exit the program
 * @param msg the error message to print
 */ 
void die(const char *msg);

/**
 * Check if the system is big endian
 * @return  1 if the system is big endian, 0 otherwise
 */ 
int is_bigendian();

/**
 * Reverse the bytes of a 64-bit integer
 * @param x the integer to reverse
 * @return  the reversed integer
 */ 
uint64_t reverse_bytes(uint64_t x);

/**
 * Convert a 64-bit integer from host byte order to big endian
 * @param x the integer to convert
 * @return  the converted integer
 */ 
uint64_t host_to_be64(uint64_t x);

/**
 * Convert a 64-bit integer from big endian to host byte order
 * @param x the integer to convert
 * @return  the converted integer
 */ 
uint64_t be64_to_host(uint64_t x);

#endif /* UTILS_H */