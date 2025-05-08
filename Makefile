# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall

# Source files
SRC = src/main.c src/io.c src/utils.c

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
