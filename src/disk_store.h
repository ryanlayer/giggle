#ifndef __DISK_STORE_H__
#define __DISK_STORE_H__

#include <stdint.h>
#include <stdio.h>

/**
 * @brief The interface to on-disk storage
 *
 * 
 * Files are stored using two data structures. Serialized data elements are
 * stored sequentially in the data_file (*.dat) and offsets is an array of
 * the last offset for each stored element in the data file. The size of the
 * element and its start position can be inferred by the previous element in the
 * offset array.  The offset array is stored in the index_file (*.idx).
 *
 * index_file: 
 *   file_header (might be absent):
 *                0-55 (7B) : GIGLIDX (File identifier/marker)
 *               56-63 (1B) : Compression Method
 *               64-71 (1B) : Compression Level
 *               72-79 (1B) : Extra
 *   data:
 *              80-111 (4B) : size
 *             112-143 (4B) : num
 *      144-... (8B * size) : offsets
 *      ...-... (8B * size) : uncompressed_sizes // if compression is used
 *
 * data_file:
 *   file_header (might be absent):
 *                 0-55 (7B) : GIGLDAT (File identifier/marker)
 *                56-63 (1B) : Compression Method
 *                64-71 (1B) : Flag
 *                72-79 (1B) : Extra
 *   data:
 *             80-offsets[0] : 1st element
 *     offsets[0]-offsets[1] : 2nd element
 *   ...
 *   offsets[i-1]-offsets[i] : nth element
 *   ...
 */

struct disk_store
{
    char *index_file_name; //!< Name of file containing offsets
    char *data_file_name; //!< Name of file containing data
    FILE *index_fp; //!< File pointer to the open handle containing offsets
    FILE *data_fp; //!< File pointer to the open handle containing data
    uint32_t size; //!< Amount of allocated space in offsets array
    uint32_t num; //!< Number of elements stored
    uint64_t index_start_offset; //!< End of header file position in offsets
    uint64_t data_start_offset; //!< End of header file position in data
    uint64_t *offsets; //!< Array of data end offsets stored on disk
    uint32_t *uncompressed_sizes; //!< Array of uncompressed_sizes of data stored on disk
    uint8_t is_compressed; //!< Is the data stored in data file compressed?
    struct disk_file_header *file_header; //!< File header
};

/**
 * @brief Initialize a new disk store
 */
struct disk_store *disk_store_init(uint32_t size,
                                   FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name);

/**
 * @brief Open an existing disk store
 */
struct disk_store *disk_store_load(FILE **index_fp,
                                   char *index_file_name,
                                   FILE **data_fp,
                                   char *data_file_name);
/*
 * @brief Write the offsets file to disk
 */
void disk_store_sync(struct disk_store *ds);

/**
 * @brief Free up memory associated with the disk store
 */
void disk_store_destroy(struct disk_store **ds);

/**
 * @brief Add new elements to the store.
 *
 * This writes the element to disk and updates the in-memory version of
 * offsets, but does not write offsets to disk. disk_store_sync MUST be called
 * to save any changes.
 */
uint32_t disk_store_append(struct disk_store *ds, void *data, uint64_t size);

/**
 * @brief Get serialized elements stored in the store.
 */
void *disk_store_get(struct disk_store *ds, uint32_t id, uint64_t *size);

#endif
