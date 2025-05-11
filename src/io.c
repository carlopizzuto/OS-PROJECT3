#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include "constants.h"
#include "utils.h"
#include "io.h"

int io_file_exists(const char *filename) {
    // check if file exists
    return access(filename, F_OK) != -1;
}

int io_open(const char *filename, int flags) {
    // open file (mode 0644 if created)
    return open(filename, flags, 0644);
}

void io_close(int fd) {
    // close file
    close(fd);
}

int io_read_header(int fd, BTHeader *header) {
    // create buffer of size BLOCK_SIZE
    uint8_t buf[BLOCK_SIZE];
    // read header into buffer
    ssize_t n = pread(fd, buf, BLOCK_SIZE, 0);
    if (n != BLOCK_SIZE) return -1;
    
    // parse header
    uint64_t temp;
    // magic
    memcpy(&temp, buf, sizeof(temp));
    header->magic = be64_to_host(temp);
    // root block
    memcpy(&temp, buf + 8, sizeof(temp));
    header->root_block = be64_to_host(temp);
    // next free block
    memcpy(&temp, buf + 16, sizeof(temp));
    header->next_free_block = be64_to_host(temp);

    return 0;
}

int io_write_header(int fd, const BTHeader *header) {
    uint8_t buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);

    // store header
    uint64_t temp = host_to_be64(header->magic);
    // magic
    memcpy(buf, &temp, sizeof(temp));
    // root block
    temp = host_to_be64(header->root_block);
    memcpy(buf + 8, &temp, sizeof(temp));
    // next free block
    temp = host_to_be64(header->next_free_block);
    memcpy(buf + 16, &temp, sizeof(temp));

    // write to file
    ssize_t n = pwrite(fd, buf, BLOCK_SIZE, 0);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

int io_read_node(int fd, uint64_t block_id, void *buf) {
    // calculate offset
    off_t offset = block_id * BLOCK_SIZE;
    // read node into buffer
    ssize_t n = pread(fd, buf, BLOCK_SIZE, offset);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

int io_write_node(int fd, uint64_t block_id, const void *buf) {
    // calculate offset
    off_t offset = block_id * BLOCK_SIZE;
    // write node to file
    ssize_t n = pwrite(fd, buf, BLOCK_SIZE, offset);
    return (n == BLOCK_SIZE) ? 0 : -1;
}
