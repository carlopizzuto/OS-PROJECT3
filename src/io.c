#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "io.h"
#include "utils.h"

int create_index_file(const char *filename) {
    // Check if index file already exists
    if (access(filename, F_OK) != -1) {
        return ERROR_FILE_EXISTS;
    }

    // Create index file
    FILE *index_file = fopen(filename, "wb");
    if (index_file == NULL) {
        return ERROR_IO;
    }

     // Initialize header
    unsigned char buf[BLOCK_SIZE] = {0};        // zero-filled
    memcpy(buf + 0x00, MAGIC_NUMBER, 8);        // magic num
    *(uint64_t*)(buf + 0x08) = host_to_be64(0); // root = 0  (empty)
    *(uint64_t*)(buf + 0x10) = host_to_be64(1); // next = 1  (block #1)

    // Write header to file
    if (fwrite(buf, BLOCK_SIZE, 1, index_file) != 1) {
        fclose(index_file);
        return ERROR_IO;
    }

    fflush(index_file);
    fclose(index_file);

    return SUCCESS;
}
