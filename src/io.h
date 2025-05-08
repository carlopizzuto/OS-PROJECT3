#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdio.h>

#include "constants.h"

/**
 * Functions for managing index files
 */

/**
 * Create a B-tree index file.
 * @param filename  Path to the index file.
 * @return          0 on success, -1 on error.
 */
int create_index_file(const char *filename);

#endif /* IO_H */
