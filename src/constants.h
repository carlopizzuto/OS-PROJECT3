#ifndef CONSTANTS_H
#define CONSTANTS_H

/**
 * Constants for project
 */

// Size of each block (bytes)
#define BLOCK_SIZE      512

// Magic number to identify index files
#define MAGIC_NUMBER    "4348PRJ3"

// B-tree minimum degree (t)
#define DEGREE          10

// Maximum keys and children per node
#define MAX_KEYS        (2 * DEGREE - 1)
#define MAX_CHILDREN    (2 * DEGREE)

// Status codes
#define SUCCESS             0
#define ERROR_FILE_EXISTS   1
#define ERROR_IO            2

#endif /* CONSTANTS_H */