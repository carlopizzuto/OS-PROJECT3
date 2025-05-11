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

// Node of a B-tree
typedef struct {
    uint64_t block_id;               // block id this node is stored in (8 bytes)
    uint64_t parent_id;              // block id of parent (0 if root) (8 bytes)
    uint64_t n;                      // number of key/value pairs (8 bytes)
    uint64_t keys[MAX_KEYS];         // keys array (19 * 8 bytes = 152 bytes)
    uint64_t values[MAX_KEYS];       // values array (19 * 8 bytes = 152 bytes)
    uint64_t children[MAX_CHILDREN]; // child pointers (20 * 8 bytes = 160 bytes)
    uint8_t  pad[BLOCK_SIZE - (8 + 8 + 8 + MAX_KEYS*8 + MAX_KEYS*8 + MAX_CHILDREN*8)]; // padding
} BTNode;

// helpers to read/write nodes
static void read_node(BTree *t, uint64_t id, BTNode *node) {
    if (io_read_node(t->fd, id, node) < 0) die("io_read_node");

    // convert from big-endian to host endianness
    node->block_id = be64_to_host(node->block_id);
    node->parent_id = be64_to_host(node->parent_id);
    node->n = be64_to_host(node->n);
    // keys and values
    for (int i = 0; i < MAX_KEYS; i++) {
        node->keys[i] = be64_to_host(node->keys[i]);
        node->values[i] = be64_to_host(node->values[i]);
    }
    // children
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = be64_to_host(node->children[i]);
    }
}

static void write_node(BTree *t, uint64_t id, BTNode *node) {
    // create a copy of the node to be modified for storage
    BTNode node_be = *node;
    
    // convert from host to big-endian
    node_be.block_id = host_to_be64(node->block_id);
    node_be.parent_id = host_to_be64(node->parent_id);
    node_be.n = host_to_be64(node->n);
    // keys and values
    for (int i = 0; i < MAX_KEYS; i++) {
        node_be.keys[i] = host_to_be64(node->keys[i]);
        node_be.values[i] = host_to_be64(node->values[i]);
    }
    // children
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node_be.children[i] = host_to_be64(node->children[i]);
    }
    // write to file
    if (io_write_node(t->fd, id, &node_be) < 0) die("io_write_node");
}

// allocate a fresh block
static uint64_t alloc_node(BTree *t) {
    // update the header to point to the next free block
    uint64_t id = t->hdr.next_free_block++;
    // return the block id
    return id;
}

// forward declarations
static void split_child(BTree *t, uint64_t parent_id, int idx);
static void insert_nonfull(BTree *t, uint64_t node_id, uint64_t key, uint64_t value);


BTree* bt_create(const char *filename) {
    // check if file exists
    if (io_file_exists(filename)) die("file already exists");

    // allocate memory for the BTree structure
    BTree *t = calloc(1, sizeof(*t));
    if (!t) die("calloc");

    // open the file for reading and writing
    t->fd = io_open(filename, O_RDWR|O_CREAT);
    if (t->fd < 0) die("io_open");

    // initialize header
    memcpy(&t->hdr.magic, MAGIC_NUMBER, 8);
    t->hdr.root_block = 1;
    t->hdr.next_free_block = 2;
    // write the header
    if (io_write_header(t->fd, &t->hdr) < 0) die("io_write_header");

    // create empty root node
    BTNode root = {0};
    root.block_id = 1;
    root.parent_id = 0; // root has no parent
    root.n = 0;
    // write the root node
    write_node(t, 1, &root);

    // return the BTree structure
    return t;
}

BTree* bt_open(const char *filename) {
    // allocate memory for the BTree structure
    BTree *t = calloc(1, sizeof(*t));
    if (!t) die("calloc");

    // check if file exists
    if (!io_file_exists(filename)) die("file does not exist");

    // open the file for reading and writing
    t->fd = io_open(filename, O_RDWR);
    if (t->fd < 0) die("io_open");

    // read the header
    if (io_read_header(t->fd, &t->hdr) < 0) die("io_read_header");

    // check magic
    char magic_check[9] = {0};
    memcpy(magic_check, &t->hdr.magic, 8);
    if (strcmp(magic_check, MAGIC_NUMBER) != 0) die("invalid B-tree file");

    // return the BTree structure
    return t;
}

void bt_close(BTree *t) {
    // persist header
    if (io_write_header(t->fd, &t->hdr) < 0) perror("io_write_header");

    // close file
    io_close(t->fd);
    // free memory
    free(t);
}

int bt_insert(BTree *t, uint64_t key, uint64_t value) {
    // read root
    BTNode root;
    read_node(t, t->hdr.root_block, &root);

    // if root is full, split
    if (root.n == MAX_KEYS) {
        // allocate new root
        uint64_t old_root_id = t->hdr.root_block;
        uint64_t new_root_id = alloc_node(t);

        // create new root
        BTNode new_root = {0};
        new_root.block_id = new_root_id; // set block id
        new_root.parent_id = 0; // new root has no parent
        new_root.n = 0; // no keys or values
        new_root.children[0] = old_root_id; // set first child to old root

        // update old root's parent_id
        root.parent_id = new_root_id;
        write_node(t, old_root_id, &root);

        // write new root
        write_node(t, new_root_id, &new_root);
        t->hdr.root_block = new_root_id;

        // split old root
        split_child(t, new_root_id, 0);

        // insert nonfull
        insert_nonfull(t, new_root_id, key, value);
    } else {
        // insert nonfull
        insert_nonfull(t, t->hdr.root_block, key, value);
    }
    return 0;
}

static void split_child(BTree *t, uint64_t parent_id, int idx) {
    // declare nodes
    BTNode parent, child, sibling;
    // read parent
    read_node(t, parent_id, &parent);
    // read child
    uint64_t child_id = parent.children[idx];
    read_node(t, child_id, &child);

    // allocate sibling & set to 0
    uint64_t sib_id = alloc_node(t);
    memset(&sibling, 0, sizeof(sibling));

    // set sibling id, parent id, and n
    sibling.block_id = sib_id;
    sibling.parent_id = parent_id;
    sibling.n = DEGREE - 1;

    // move keys and values
    for (int j = 0; j < DEGREE-1; j++) {
        sibling.keys[j] = child.keys[j + DEGREE];
        sibling.values[j] = child.values[j + DEGREE];
        // zero out the moved keys/values in the child
        child.keys[j + DEGREE] = 0;
        child.values[j + DEGREE] = 0;
    }

    // move children if not leaf
    if (child.children[0] != 0) { // check if it's an internal node
        for (int j = 0; j < DEGREE; j++) {
            sibling.children[j] = child.children[j + DEGREE];
            // zero out the moved children in the child
            child.children[j + DEGREE] = 0;
        }
    }

    // set child n
    child.n = DEGREE - 1;

    // shift parent entries
    for (int j = parent.n; j > idx; j--) {
        parent.children[j+1] = parent.children[j];
        parent.keys[j] = parent.keys[j-1];
        parent.values[j] = parent.values[j-1];
    }

    // set parent children and keys
    parent.children[idx+1] = sib_id;
    parent.keys[idx] = child.keys[DEGREE-1];
    parent.values[idx] = child.values[DEGREE-1];
    parent.n++;

    // zero out the moved keys/values in the child
    child.keys[DEGREE-1] = 0;
    child.values[DEGREE-1] = 0;

    // write back
    write_node(t, child_id, &child);
    write_node(t, sib_id, &sibling);
    write_node(t, parent_id, &parent);
}

static void insert_nonfull(BTree *t, uint64_t node_id, uint64_t key, uint64_t value) {
    // declare node
    BTNode node;
    // read node
    read_node(t, node_id, &node);

    // set i to last key index
    int i = node.n - 1;

    // if leaf node
    if (node.children[0] == 0) { // this is a leaf node
        // shift and insert key and value
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
        // find child
        while (i >= 0 && key < node.keys[i]) i--;
        i++;
        // load child
        BTNode child;
        read_node(t, node.children[i], &child);
        if (child.n == MAX_KEYS) {
            split_child(t, node_id, i);
            // decide which of the two to descend
            read_node(t, node_id, &node);
            if (key > node.keys[i]) i++;
        }
        insert_nonfull(t, node.children[i], key, value);
    }
}