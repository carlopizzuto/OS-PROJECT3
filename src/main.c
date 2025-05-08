#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


#define BLOCK_SIZE 512


int is_bigendian() {
    int x = 1;
    return ((uint8_t *)&x)[0] != 1;
}

uint64_t reverse_bytes(uint64_t x) {
    uint8_t dest[sizeof(uint64_t)];
    uint8_t *source = (uint8_t*)&x;

    for(int c = 0; c < sizeof(uint64_t); c++) {
        dest[c] = source[sizeof(uint64_t)-c-1];
    }

    return *(uint64_t *)dest;
}

uint64_t host_to_be64(uint64_t x) {
    if (is_bigendian()) {
        return x;
    }
    return reverse_bytes(x);
}

int main(int argc, char *argv[]) {
    // initial input error handling
    if (argc < 3) {
        fprintf(stderr, "Usage: ./main <command> <index_file> [arguments]\n");
        fprintf(stderr, "Valid commands: create, insert, search, load, print, extract\n");
        exit(EXIT_FAILURE);
    }

    const char *command = argv[1];
    const char *index_file_path = argv[2];

    if (strcmp(command, "create") == 0) {
        printf("creating index file...\n");  

        // check if command is called with correct number of arguments
        if (argc != 3) {
            fprintf(stderr, "Usage: ./main create <index_file>\n");
            exit(EXIT_FAILURE);
        }

        // check if index file already exists
        if (access(index_file_path, F_OK) != -1) {
            fprintf(stderr, "Index file already exists\n");
            exit(EXIT_FAILURE);
        }

        // create index file
        FILE *index_file = fopen(index_file_path, "wb");

        // check if index file was created successfully
        if (index_file == NULL) {
            fprintf(stderr, "Failed to create index file\n");
            exit(EXIT_FAILURE);
        }

        // initialize header
        unsigned char buf[BLOCK_SIZE] = {0};           // zero‑filled
        memcpy(buf+0x00, "4348PRJ3", 8);               // magic num
        *(uint64_t*)(buf+0x08) = host_to_be64(0);           // root = 0  (empty)
        *(uint64_t*)(buf+0x10) = host_to_be64(1);           // next = 1  (block #1)

        // write header to index file
        if (fwrite(buf, BLOCK_SIZE, 1, index_file) != 1) {
            fprintf(stderr, "Failed to write header to index file\n");
            exit(EXIT_FAILURE);
        }
        fflush(index_file);

        // close index file
        fclose(index_file);

        // print success message
        printf("index file created successfully\n");
    }
    else if (strcmp(command, "insert") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Usage: ./main insert <index_file> <key> <value>\n");
            exit(EXIT_FAILURE);
        }
        printf("inserting data into index file...\n");
        
    }
    else if (strcmp(command, "search") == 0) {
        if (argc != 5) {
            fprintf(stderr, "Usage: ./main search <index_file> <key>\n");
            exit(EXIT_FAILURE);
        }
        printf("searching for data in index file...\n");

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