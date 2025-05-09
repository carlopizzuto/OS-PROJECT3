#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Constants for project
 */

// Size of each block (bytes)
#define BLOCK_SIZE      512

// Size of the header (bytes)
#define HEADER_SIZE     24

// Magic number to identify index files
#define MAGIC_NUMBER    "4348PRJ3"

// B-tree minimum degree (t)
#define DEGREE          10

// Maximum keys and children per node
#define MAX_KEYS        (2 * DEGREE - 1)
#define MAX_CHILDREN    (2 * DEGREE)

// B-tree header structure
typedef struct {
    uint64_t magic;           // Magic number for file validation (8 bytes)
    uint64_t root_block;      // Block number of the root node (8 bytes)
    uint64_t next_free_block; // Next available block number for allocation (8 bytes)
    uint8_t  reserved[BLOCK_SIZE - 24]; // Padding to fill header block
} BTHeader;

// Status codes
#define SUCCESS             0
#define ERROR_FILE_EXISTS   1
#define ERROR_IO            2

#endif /* CONSTANTS_H */
