#ifndef __DISK_FILE_HEADER_H__
#define __DISK_FILE_HEADER_H__

#include <stdint.h>
#include <stdio.h>

#define GIGGLE_FILE_MARKER_LENGTH 7
#define GIGGLE_INDEX_FILE_MARKER "GIGLIDX"
#define GIGGLE_DATA_FILE_MARKER "GIGLDAT"

struct disk_file_header {
    uint8_t compression_method, flag, extra;
};

/**
 * @brief Create a new disk file header
 */
struct disk_file_header *new_disk_file_header(uint8_t compression_method);

/**
 * @brief Write disk file header to file
 */
void write_disk_file_header(char *file_marker, struct disk_file_header *h, FILE *fp, char *file_name);

/**
 * @brief Read disk file header
 */
struct disk_file_header *read_disk_file_header(FILE *fp, char *file_name, char *expected_file_marker);

#endif
