# B-tree Index Implementation

A C implementation of a B-tree data structure for efficient storage and retrieval of key-value pairs.

## Overview

This project implements a B-tree index that can store 64-bit key-value pairs. The B-tree is stored on disk in an index file, allowing for persistent storage and retrieval of large datasets.

## Prerequisites

- GCC compiler
- Make
- [Extra] Python 3 (for using the seed.py script)

## Building the Project

To build the project, simply run:

```bash
make
```

This will compile the source code and create the executable named `main`.

To clean the build files:

```bash
make clean
```

## Usage

The program provides several commands for interacting with B-tree index files:

### Create a New Index File

```bash
./main create <index_file>
```

### Insert a Key-Value Pair

```bash
./main insert <index_file> <key> <value>
```

### Search for a Key

```bash
./main search <index_file> <key>
```

### Load Data from a CSV File

```bash
./main load <index_file> <csv_file>
```

### Print B-tree Structure

```bash
./main print <index_file>
```

### Extract All Key-Value Pairs to a CSV File

```bash
./main extract <index_file> <csv_file>
```

### Example Usage

```bash
% make      
gcc -Wall -c src/main.c -o src/main.o
gcc -Wall -c src/utils.c -o src/utils.o
gcc -Wall -c src/io.c -o src/io.o
gcc -Wall -c src/btree.c -o src/btree.o
gcc -Wall -o main src/main.o src/utils.o src/io.o src/btree.o
% ./main create data/test.idx
index file created successfully
% ./main load data/test.idx data/data.csv
Loaded 10 key-value pairs from CSV file
% ./main search data/test.idx 9            
key found in b-tree with value 9
% ./main print data/test.idx             
B-Tree Root Block: 2
B-Tree Next Free Block: 6
----------------------------
L0 Node[2] (parent=0, n=3): (5,5) (7,7) (9,9) 
  └── L1 Node[1] (parent=2, n=3): (2,2) (3,3) (4,4) 
  └── L1 Node[4] (parent=2, n=1): (6,6) 
  └── L1 Node[3] (parent=2, n=1): (8,8) 
  └── L1 Node[5] (parent=2, n=2): (10,10) (11,11) 
% ./main insert data/test.idx 12 12
data inserted into b-tree
% ./main print data/test.idx       
B-Tree Root Block: 6
B-Tree Next Free Block: 8
----------------------------
L0 Node[6] (parent=0, n=1): (7,7) 
  └── L1 Node[2] (parent=6, n=1): (5,5) 
    └── L2 Node[1] (parent=2, n=3): (2,2) (3,3) (4,4) 
    └── L2 Node[4] (parent=2, n=1): (6,6) 
  └── L1 Node[7] (parent=6, n=1): (9,9) 
    └── L2 Node[3] (parent=7, n=1): (8,8) 
    └── L2 Node[5] (parent=7, n=3): (10,10) (11,11) (12,12) 
% ./main extract data/test.idx data/out.csv
extracting data from index file...
Extracted 11 key-value pairs to CSV file
```

## Data Format

The CSV files used for loading and extracting data should have the following format:
- Each line contains a key-value pair separated by a comma
- Both key and value should be 64-bit integers
- A line can become a comment if it starts with a `#`

Example CSV content:
```
# This is a comment
123,456
789,101112
```


## Project Structure

- `src/`: Contains all source code files
  - `main.c`: Main program entry point
  - `btree.c/h`: B-tree implementation
  - `io.c/h`: Disk I/O operations for index file
  - `utils.c/h`: Utility functions
  - `constants.h`: Constants and error codes
- `data/`: Directory for storing index files and test data
- `Makefile`: Build configuration
