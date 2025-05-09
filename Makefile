# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall

# Source files
SRC = src/main.c src/utils.c src/io.c src/btree.c

# Object files
OBJ = $(SRC:.c=.o)

# Default target
main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

# Pattern rule for any .o ← .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJ) main

# Phony target
.PHONY: clean
