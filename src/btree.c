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

// helper to read node from file
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

// helper to write node to file
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

// helper to allocate a fresh block
static uint64_t alloc_node(BTree *t) {
    // update the header to point to the next free block
    uint64_t id = t->hdr.next_free_block++;
    // return the block id
    return id;
}

// forward declarations
static void split_child(BTree *t, uint64_t parent_id, int idx);
static void insert_nonfull(BTree *t, uint64_t node_id, uint64_t key, uint64_t value);
static void print_node(BTree *t, uint64_t node_id, int level);
static int extract_node(BTree *t, FILE *file, uint64_t node_id, int *pair_count);


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

int bt_search(BTree *t, uint64_t key, uint64_t *value) {
    // start from the root node
    BTNode node;
    uint64_t current_node_id = t->hdr.root_block;
    
    while (1) {
        // read the current node
        read_node(t, current_node_id, &node);

        // search for key in the current node
        int i = 0;
        while (i < node.n && key > node.keys[i]) {
            i++;
        }

        // check if we found the key
        if (i < node.n && key == node.keys[i]) {
            // if value pointer is provided, store the value
            if (value != NULL) {
                *value = node.values[i];
            }
            return SUCCESS; // key found
        }

        // if current node has no children
        if (node.children[0] == 0) { // it is a leaf node 
            return ERROR_KEY_NOT_FOUND; // key not found
        }

        // continue search in the appropriate child
        current_node_id = node.children[i];
    }
}

int bt_load(BTree *t, const char *csv_file) {
    // open the csv file for reading
    FILE *file = fopen(csv_file, "r");
    if (file == NULL) {
        perror("Error opening CSV file");
        return -1;
    }

    // buffer for reading lines from the file
    char line[1024];
    int line_count = 0;
    int success_count = 0;

    // process each line in the csv file
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        
        // skip empty lines or lines starting with #
        if (line[0] == '\n' || line[0] == '#') {
            continue;
        }

        // parse the csv line for key and value
        char *token = strtok(line, ",");
        if (token == NULL) {
            fprintf(stderr, "Error parsing line %d: Invalid format\n", line_count);
            continue;
        }

        // convert the key string to a 64-bit unsigned integer
        uint64_t key = strtoull(token, NULL, 10);
        
        // get the value part of the csv line
        token = strtok(NULL, ",\n");
        if (token == NULL) {
            fprintf(stderr, "Error parsing line %d: Missing value\n", line_count);
            continue;
        }

        // convert the value string to a 64-bit unsigned integer
        uint64_t value = strtoull(token, NULL, 10);
        
        // insert the key-value pair into the b-tree
        int result = bt_insert(t, key, value);
        if (result != SUCCESS) {
            fprintf(stderr, "Error inserting key-value pair (%llu, %llu) at line %d\n", 
                    (unsigned long long)key, (unsigned long long)value, line_count);
        } else {
            success_count++;
        }
    }

    // close the file when done
    fclose(file);
    
    // print summary and return success
    printf("Loaded %d key-value pairs from CSV file\n", success_count);
    return SUCCESS;
}

int bt_extract(BTree *t, const char *csv_file) {
    // open the csv file for writing
    FILE *file = fopen(csv_file, "w");
    if (file == NULL) {
        perror("Error opening CSV file for writing");
        return -1;
    }

    // write a header comment
    fprintf(file, "# Key-value pairs extracted from B-tree\n");
    fprintf(file, "# Format: key,value\n");
    
    // counter for the number of key-value pairs extracted
    int pair_count = 0;

    // start traversal from the root node
    int result = extract_node(t, file, t->hdr.root_block, &pair_count);

    // close the file
    fclose(file);

    // print summary and return success
    printf("Extracted %d key-value pairs to CSV file\n", pair_count);
    return result;
}

void bt_print(BTree *t) {
    printf("B-Tree Root Block: %llu\n", (unsigned long long)t->hdr.root_block);
    printf("B-Tree Next Free Block: %llu\n", (unsigned long long)t->hdr.next_free_block);
    printf("----------------------------\n");
    
    // start printing from the root
    print_node(t, t->hdr.root_block, 0);
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

    // if child has children
    if (child.children[0] != 0) { 
        // for each grandchild
        for (int j = 0; j < DEGREE; j++) {
            // move grandchild
            sibling.children[j] = child.children[j + DEGREE];
            
            // if grandchild has children
            if (sibling.children[j] != 0) {
                // update the parent pointer of the moved grandchild
                BTNode moved_child;
                read_node(t, sibling.children[j], &moved_child);
                moved_child.parent_id = sib_id;
                write_node(t, sibling.children[j], &moved_child);
            }
            
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
    // load the initial node
    BTNode node;
    read_node(t, node_id, &node);

    // set i to last key index
    int i = node.n - 1;

    // if node is leaf
    if (node.children[0] == 0) {
        // shift keys and values to make room for new entry
        while (i >= 0 && key < node.keys[i]) {
            node.keys[i+1] = node.keys[i];
            node.values[i+1] = node.values[i];
            i--;
        }

        // insert the new key and value
        node.keys[i+1] = key;
        node.values[i+1] = value;
        node.n++;

        // write back the updated node
        write_node(t, node_id, &node);
    } else { // node has children
        // find the child to descend into
        while (i >= 0 && key < node.keys[i]) i--;
        i++;

        // load the child node
        BTNode child;
        read_node(t, node.children[i], &child);

        // if child is full
        if (child.n == MAX_KEYS) {
            // split it
            split_child(t, node_id, i);

            // reload node after split and determine which child to descend into
            read_node(t, node_id, &node);
            if (key > node.keys[i]) i++;
        }
        // recursively insert into the appropriate child
        insert_nonfull(t, node.children[i], key, value);
    }
}

static void print_node(BTree *t, uint64_t node_id, int level) {
    // load current node
    BTNode node;
    read_node(t, node_id, &node);
    
    // print indentation based on level
    for (int i = 0; i < level; i++) {
        printf("  ");
    }

    if (level > 0) {
        printf("└── ");
    }

    printf("L%d ", level);
    
    // print node information
    printf("Node[%llu] (parent=%llu, n=%llu): ", 
           (unsigned long long)node.block_id,
           (unsigned long long)node.parent_id,
           (unsigned long long)node.n);
    
    // print keys and values
    for (int i = 0; i < node.n; i++) {
        printf("(%llu,%llu) ", 
               (unsigned long long)node.keys[i], 
               (unsigned long long)node.values[i]);
    }
    printf("\n");
    
    // if node has children
    if (node.children[0] != 0) {
        // for each child
        for (int i = 0; i <= node.n; i++) {
            // if child exists
            if (node.children[i] != 0) {
                // recursively print child
                print_node(t, node.children[i], level + 1);
            }
        }
    }
}

static int extract_node(BTree *t, FILE *file, uint64_t node_id, int *pair_count) {
    // load the current node
    BTNode node;
    read_node(t, node_id, &node);

    // if this is a leaf node
    if (node.children[0] == 0) {
        // for each key-value pair
        for (int i = 0; i < node.n; i++) {
            // write key-value pair to csv file
            fprintf(file, "%llu,%llu\n", 
                    (unsigned long long)node.keys[i], 
                    (unsigned long long)node.values[i]);    
            (*pair_count)++;
        }
        return SUCCESS;
    }

    // for internal nodes, traverse children and write keys in order
    for (int i = 0; i < node.n; i++) {
        // if left child exists
        if (node.children[i] != 0) {
            // traverse the left child
            extract_node(t, file, node.children[i], pair_count);
        }
        
        // write the current key-value pair
        fprintf(file, "%llu,%llu\n", 
                (unsigned long long)node.keys[i], 
                (unsigned long long)node.values[i]);
        (*pair_count)++;
    }

    // if rightmost child exists
    if (node.children[node.n] != 0) {
        // traverse the rightmost child
        extract_node(t, file, node.children[node.n], pair_count);
    }

    return SUCCESS;
}