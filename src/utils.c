#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "utils.h"

void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

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

uint64_t be64_to_host(uint64_t x) {
    if (is_bigendian()) {
        return x;
    }
    return reverse_bytes(x);
}