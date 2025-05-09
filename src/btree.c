#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "btree.h"
#include "io.h"
#include "constants.h"
#include "utils.h"

struct BTree {
    int      fd;
    BTHeader hdr;
};

// In-memory node representation matching on-disk layout
typedef struct {
    uint64_t block_id;               // Block ID this node is stored in (8 bytes)
    uint64_t parent_id;              // Block ID of parent (0 if root) (8 bytes)
    uint64_t n;                      // Number of key/value pairs (8 bytes)
    uint64_t keys[MAX_KEYS];         // Keys array (19 * 8 bytes = 152 bytes)
    uint64_t values[MAX_KEYS];       // Values array (19 * 8 bytes = 152 bytes)
    uint64_t children[MAX_CHILDREN]; // Child pointers (20 * 8 bytes = 160 bytes)
    uint8_t  pad[BLOCK_SIZE - (8 + 8 + 8 + MAX_KEYS*8 + MAX_KEYS*8 + MAX_CHILDREN*8)]; // Padding
} BNode;

// Helpers to read/write nodes
static void read_node(BTree *t, uint64_t id, BNode *node) {
    if (io_read_node(t->fd, id, node) < 0) die("io_read_node");

    // Convert from big-endian to host endianness
    node->block_id = be64_to_host(node->block_id);
    node->parent_id = be64_to_host(node->parent_id);
    node->n = be64_to_host(node->n);
    
    for (int i = 0; i < MAX_KEYS; i++) {
        node->keys[i] = be64_to_host(node->keys[i]);
        node->values[i] = be64_to_host(node->values[i]);
    }
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = be64_to_host(node->children[i]);
    }
}

static void write_node(BTree *t, uint64_t id, BNode *node) {
    // Create a copy of the node to be modified for storage
    BNode node_be = *node;
    
    // Convert from host to big-endian
    node_be.block_id = host_to_be64(node->block_id);
    node_be.parent_id = host_to_be64(node->parent_id);
    node_be.n = host_to_be64(node->n);
    
    for (int i = 0; i < MAX_KEYS; i++) {
        node_be.keys[i] = host_to_be64(node->keys[i]);
        node_be.values[i] = host_to_be64(node->values[i]);
    }
    
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node_be.children[i] = host_to_be64(node->children[i]);
    }
    
    if (io_write_node(t->fd, id, &node_be) < 0) die("io_write_node");
}

// Allocate a fresh block
static uint64_t alloc_node(BTree *t) {
    uint64_t id = t->hdr.next_free_block++;
    return id;
}

// Forward declarations
static void split_child(BTree *t, uint64_t parent_id, int idx);
static void insert_nonfull(BTree *t, uint64_t node_id, uint64_t key, uint64_t value);


BTree* bt_create(const char *filename) {
    BTree *t = calloc(1, sizeof(*t));
    if (!t) die("calloc");
    t->fd = io_open(filename, O_RDWR|O_CREAT);
    if (t->fd < 0) die("io_open");
    // Initialize header
    memcpy(&t->hdr.magic, MAGIC_NUMBER, 8);
    t->hdr.root_block = 1;
    t->hdr.next_free_block = 2;
    if (io_write_header(t->fd, &t->hdr) < 0) die("io_write_header");
    // Create empty root node
    BNode root = {0};
    root.block_id = 1;
    root.parent_id = 0; // Root has no parent
    root.n = 0;
    write_node(t, 1, &root);
    return t;
}

BTree* bt_open(const char *filename) {
    BTree *t = calloc(1, sizeof(*t));
    if (!t) die("calloc");
    t->fd = io_open(filename, O_RDWR);
    if (t->fd < 0) die("io_open");
    if (io_read_header(t->fd, &t->hdr) < 0) die("io_read_header");
    char magic_check[9] = {0};
    memcpy(magic_check, &t->hdr.magic, 8);
    if (strcmp(magic_check, MAGIC_NUMBER) != 0) die("invalid B-tree file");
    return t;
}

void bt_close(BTree *t) {
    // Persist header
    if (io_write_header(t->fd, &t->hdr) < 0) perror("io_write_header");
    io_close(t->fd);
    free(t);
}

int bt_insert(BTree *t, uint64_t key, uint64_t value) {
    // Read root
    BNode root;
    read_node(t, t->hdr.root_block, &root);
    if (root.n == MAX_KEYS) {
        // Root full => split
        uint64_t old_root_id = t->hdr.root_block;
        uint64_t new_root = alloc_node(t);
        BNode nr = {0};
        nr.block_id = new_root;
        nr.parent_id = 0; // New root has no parent
        nr.n = 0;
        nr.children[0] = old_root_id; // Set first child to old root
        write_node(t, new_root, &nr);
        t->hdr.root_block = new_root;
        split_child(t, new_root, 0);
        insert_nonfull(t, new_root, key, value);
    } else {
        insert_nonfull(t, t->hdr.root_block, key, value);
    }
    return 0;
}

static void split_child(BTree *t, uint64_t parent_id, int idx) {
    BNode parent, child, sibling;
    read_node(t, parent_id, &parent);
    uint64_t child_id = parent.children[idx];
    read_node(t, child_id, &child);
    // Allocate sibling
    uint64_t sib_id = alloc_node(t);
    memset(&sibling, 0, sizeof(sibling));
    sibling.block_id = sib_id;
    sibling.parent_id = parent_id;
    sibling.n = DEGREE - 1;
    // Move keys and values
    for (int j = 0; j < DEGREE-1; j++) {
        sibling.keys[j] = child.keys[j + DEGREE];
        sibling.values[j] = child.values[j + DEGREE];
    }
    // Move children if not leaf
    if (child.children[0] != 0) { // Check if it's an internal node
        for (int j = 0; j < DEGREE; j++)
            sibling.children[j] = child.children[j + DEGREE];
    }
    child.n = DEGREE - 1;
    // Shift parent entries
    for (int j = parent.n; j > idx; j--) {
        parent.children[j+1] = parent.children[j];
        parent.keys[j] = parent.keys[j-1];
        parent.values[j] = parent.values[j-1];
    }
    parent.children[idx+1] = sib_id;
    parent.keys[idx] = child.keys[DEGREE-1];
    parent.values[idx] = child.values[DEGREE-1];
    parent.n++;
    // Write back
    write_node(t, child_id, &child);
    write_node(t, sib_id, &sibling);
    write_node(t, parent_id, &parent);
}

static void insert_nonfull(BTree *t, uint64_t node_id, uint64_t key, uint64_t value) {
    BNode node;
    read_node(t, node_id, &node);
    int i = node.n - 1;
    if (node.children[0] == 0) { // This is a leaf node
        // Shift and insert key and value
        while (i >= 0 && key < node.keys[i]) {
            node.keys[i+1] = node.keys[i];
            node.values[i+1] = node.values[i];
            i--;
        }
        node.keys[i+1] = key;
        node.values[i+1] = value;
        node.n++;
        write_node(t, node_id, &node);
    } else {
        // Find child
        while (i >= 0 && key < node.keys[i]) i--;
        i++;
        // Load child
        BNode child;
        read_node(t, node.children[i], &child);
        if (child.n == MAX_KEYS) {
            split_child(t, node_id, i);
            // Decide which of the two to descend
            read_node(t, node_id, &node);
            if (key > node.keys[i]) i++;
        }
        insert_nonfull(t, node.children[i], key, value);
    }
}