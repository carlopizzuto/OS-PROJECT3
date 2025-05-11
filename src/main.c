#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "io.h"
#include "utils.h"
#include "btree.h"

#define BLOCK_SIZE 512

int main(int argc, char *argv[]) {
    // check if the call includes a command and index file
    if (argc < 3) {
        fprintf(stderr, "Usage: ./main <command> <index_file> [arguments]\n");
        fprintf(stderr, "Valid commands: create, insert, search, load, print, extract\n");
        exit(EXIT_FAILURE);
    }

    const char *command = argv[1];
    const char *index_file_path = argv[2];

    if (strcmp(command, "create") == 0) {
        printf("creating index file...\n");  

        // check if create is called with extra arguments
        if (argc != 3) {
            fprintf(stderr, "Usage: ./main create <index_file>\n");
            exit(EXIT_FAILURE);
        }

        // create index file
        BTree *tree = bt_create(index_file_path);
        if (tree == NULL) {
            fprintf(stderr, "Error: Failed to create index file\n");
            exit(EXIT_FAILURE);
        }

        // print success message
        printf("index file created successfully\n");
        // close index file
        bt_close(tree);
    }
    else if (strcmp(command, "insert") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Usage: ./main insert <index_file> <key> <value>\n");
            exit(EXIT_FAILURE);
        }
        printf("checking if index file exists...\n");
        if (!io_file_exists(index_file_path)) {
            fprintf(stderr, "Error: Index file does not exist\n");
            exit(EXIT_FAILURE);
        }
        printf("index file exists\n");

        printf("opening b-tree...\n");
        BTree *tree = bt_open(index_file_path);
        if (tree == NULL) {
            fprintf(stderr, "Error: Failed to open b-tree\n");
            exit(EXIT_FAILURE);
        }
        printf("b-tree opened\n");

        uint64_t key = strtoull(argv[3], NULL, 10);
        uint64_t value = strtoull(argv[4], NULL, 10);
        
        printf("inserting data into b-tree...\n");
        int result = bt_insert(tree, key, value);
        if (result != SUCCESS) {
            fprintf(stderr, "Error: Failed to insert data into b-tree\n");
            bt_close(tree);
            exit(EXIT_FAILURE);
        }

        printf("data inserted into b-tree\n");
        bt_close(tree);
    }
    else if (strcmp(command, "search") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: ./main search <index_file> <key>\n");
            exit(EXIT_FAILURE);
        }
        printf("checking if index file exists...\n");
        if (!io_file_exists(index_file_path)) {
            fprintf(stderr, "Error: Index file does not exist\n");
            exit(EXIT_FAILURE);
        }
        printf("index file exists\n");

        printf("opening b-tree...\n");
        BTree *tree = bt_open(index_file_path);
        if (tree == NULL) {
            fprintf(stderr, "Error: Failed to open b-tree\n");
            exit(EXIT_FAILURE);
        }
        printf("b-tree opened\n");
    }
    else if (strcmp(command, "load") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: ./main load <index_file> <csv_file>\n");
            exit(EXIT_FAILURE);
        }
        printf("loading data from index file...\n");
        
    }
    else if (strcmp(command, "print") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: ./main print <index_file>\n");
            exit(EXIT_FAILURE);
        }
        printf("printing data from index file...\n");
        
    }
    else if (strcmp(command, "extract") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: ./main extract <index_file> <csv_file>\n");
            exit(EXIT_FAILURE);
        }
        printf("extracting data from index file...\n");

    }
    else {
        fprintf(stderr, "Usage: ./main <command> <index_file> [arguments]\n");
        fprintf(stderr, "Valid commands: create, insert, search, load, print, extract\n");
        exit(EXIT_FAILURE);
    }


    return 0;
}