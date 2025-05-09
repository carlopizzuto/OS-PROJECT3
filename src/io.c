#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>

#include "constants.h"
#include "utils.h"
#include "io.h"

int io_file_exists(const char *filename) {
    return access(filename, F_OK) != -1;
}

int io_open(const char *filename, int flags) {
    // Open file (mode 0644 if created)
    return open(filename, flags, 0644);
}

void io_close(int fd) {
    close(fd);
}

int io_read_header(int fd, BTHeader *header) {
    uint8_t buf[BLOCK_SIZE];
    ssize_t n = pread(fd, buf, BLOCK_SIZE, 0);
    if (n != BLOCK_SIZE) return -1;
    
    // Parse header
    uint64_t temp;
    memcpy(&temp, buf, sizeof(temp));
    header->magic = be64_to_host(temp);
    memcpy(&temp, buf + 8, sizeof(temp));
    header->root_block = be64_to_host(temp);
    memcpy(&temp, buf + 16, sizeof(temp));
    header->next_free_block = be64_to_host(temp);

    return 0;
}

int io_write_header(int fd, const BTHeader *header) {
    uint8_t buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);

    // Store header
    uint64_t temp = host_to_be64(header->magic);
    memcpy(buf, &temp, sizeof(temp));
    temp = host_to_be64(header->root_block);
    memcpy(buf + 8, &temp, sizeof(temp));
    temp = host_to_be64(header->next_free_block);
    memcpy(buf + 16, &temp, sizeof(temp));

    ssize_t n = pwrite(fd, buf, BLOCK_SIZE, 0);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

int io_read_node(int fd, uint64_t block_id, void *buf) {
    off_t offset = block_id * BLOCK_SIZE;
    ssize_t n = pread(fd, buf, BLOCK_SIZE, offset);
    return (n == BLOCK_SIZE) ? 0 : -1;
}

int io_write_node(int fd, uint64_t block_id, const void *buf) {
    off_t offset = block_id * BLOCK_SIZE;
    ssize_t n = pwrite(fd, buf, BLOCK_SIZE, offset);
    return (n == BLOCK_SIZE) ? 0 : -1;
}
