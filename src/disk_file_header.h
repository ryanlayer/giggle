#ifndef __DISK_FILE_HEADER_H__
#define __DISK_FILE_HEADER_H__

#include <stdint.h>
#include <stdio.h>

#define GIGGLE_FILE_MARKER_LENGTH 7
#define GIGGLE_INDEX_FILE_MARKER "GIGLIDX"
#define GIGGLE_DATA_FILE_MARKER "GIGLDAT"
typedef unsigned long  uLong;

struct disk_file_header {
    uint8_t compression_method, compression_level, extra;
    void* (*compress)(void *data, uLong uncompressed_size, int level, uLong *compressed_size); //!< Compress function pointer
    void* (*uncompress)(void *compressed_data, uLong compressed_size, uLong uncompressed_size); //!< Uncompress function pointer
};

/**
 * @brief Set compression function pointers to the disk file header
 */
void set_compression_function_pointers(struct disk_file_header *h);

/**
 * @brief Create a new disk file header
 */
struct disk_file_header *new_disk_file_header(uint8_t compression_method, uint8_t compression_level);

/**
 * @brief Write disk file header to file
 */
void write_disk_file_header(char *file_marker, struct disk_file_header *h, FILE *fp, char *file_name);

/**
 * @brief Read disk file header
 */
struct disk_file_header *read_disk_file_header(FILE *fp, char *file_name, char *expected_file_marker);

#endif
