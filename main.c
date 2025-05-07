#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int choice;
    char username[20];
    char password[20];
    char confirm_password[20];

    if (argc < 3) {
        fprintf(stderr, "Usage: ./main <command> <index_file> [arguments]\n");
        fprintf(stderr, "Valid commands: create, insert, search, load, print, extract\n");
        exit(EXIT_FAILURE);
    }

    const char *command = argv[1];
    const char *index_file = argv[2];

    if (strcmp(command, "create") == 0) {
        if (argc != 3) {
            fprintf(stderr, "Usage: ./main create <index_file>\n");
            exit(EXIT_FAILURE);
        }
        printf("creating index file...\n");        
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