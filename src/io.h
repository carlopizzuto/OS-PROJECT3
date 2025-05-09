#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <stdio.h>

#include "constants.h"

/**
 * Functions for managing index files
 */

/**
 * Check if a file exists
 * @param filename  Path to the file
 * @return          1 if file exists, 0 otherwise
 */
int io_file_exists(const char *filename);

/**
 * Open an index file
 * @param filename  Path to the file
 * @param flags     Flags for the open call
 * @return          File descriptor on success, -1 on error
 */
int io_open(const char *filename, int flags);

/**
 * Close an index file
 * @param fd        File descriptor
 */
void io_close(int fd);

/**
 * Read the header of an index file
 * @param fd        File descriptor
 * @param header    Pointer to the header structure
 */
int io_read_header(int fd, BTHeader *header);

/**
 * Write the header of an index file
 * @param fd        File descriptor
 * @param header    Pointer to the header structure
 */
int io_write_header(int fd, const BTHeader *header);

/**
 * Read a node from an index file
 * @param fd        File descriptor
 * @param block_id  Block ID
 * @param buf       Pointer to the buffer to read the node into
 */
int io_read_node(int fd, uint64_t block_id, void *buf);

/**
 * Write a node to an index file
 * @param fd        File descriptor
 * @param block_id  Block ID
 * @param buf       Pointer to the buffer to write the node from
 */
int io_write_node(int fd, uint64_t block_id, const void *buf);

#endif /* IO_H */
