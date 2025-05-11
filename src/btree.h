#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>

/**
 * Header file for the B-tree
 */

/**
 * Opaque handle for a B-tree index.
 */
typedef struct BTree BTree;

/**
 * Create a new B-tree index file.
 * @param filename  Path to the index file to create.
 * @return          Pointer to a BTree handle, or NULL on error.
 */
BTree* bt_create(const char *filename);

/**
 * Open an existing B-tree index file.
 * @param filename  Path to the index file to open.
 * @return          Pointer to a BTree handle, or NULL on error.
 */
BTree* bt_open(const char *filename);

/**
 * Close a B-tree and free associated resources.
 * @param tree      BTree handle returned by bt_create or bt_open.
 */
void bt_close(BTree *tree);

/**
 * Insert a key-value pair into the B-tree.
 * @param tree      The BTree handle.
 * @param key       64-bit key to insert.
 * @param value     64-bit value to associate with the key.
 * @return          0 on success, non-zero on failure.
 */
int bt_insert(BTree *tree, uint64_t key, uint64_t value);

/**
 * Search for a key in the B-tree.
 * @param tree      The BTree handle.
 * @param key       64-bit key to search for.
 * @param value     Pointer to store the value associated with the key if found.
 * @return          SUCCESS if key is found, non-zero otherwise.
 */
int bt_search(BTree *tree, uint64_t key, uint64_t *value);

#endif /* BTREE_H */