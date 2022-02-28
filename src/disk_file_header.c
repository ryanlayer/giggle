#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <err.h>
#include "util.h"
#include "disk_file_header.h"

struct disk_file_header *new_disk_file_header(uint8_t compression_method) {
    
    struct disk_file_header *h = (struct disk_file_header *) 
            calloc(1, sizeof(struct disk_file_header));
    if (h == NULL)
        err(1, "calloc error in new_disk_file_header().");

    h->compression_method = compression_method;
    return h;
}

void write_disk_file_header(char *file_marker, struct disk_file_header *h, FILE *fp, char *file_name) {
    if (fwrite(file_marker, sizeof(char), GIGGLE_FILE_MARKER_LENGTH, fp) != GIGGLE_FILE_MARKER_LENGTH)
        err(1, "Could not write file_marker to '%s'.", file_name);

    if (fwrite(&(h->compression_method), sizeof(uint8_t), 1, fp) != 1)
        err(1, "Could not write compression_method to '%s'.", file_name);

    if (fwrite(&(h->flag), sizeof(uint8_t), 1, fp) != 1)
        err(1, "Could not write flag to '%s'.", file_name);

    if (fwrite(&(h->extra), sizeof(uint8_t), 1, fp) != 1)
        err(1, "Could not write extra to '%s'.", file_name);
}

struct disk_file_header *read_disk_file_header(FILE *fp, char *file_name, char *expected_file_marker) {
    char file_marker[7];
    
    size_t fr = fread(file_marker, sizeof(char), GIGGLE_FILE_MARKER_LENGTH, fp);
    if (fr != GIGGLE_FILE_MARKER_LENGTH || strcmp(expected_file_marker, file_marker) != 0) {
        fseek(fp, 0, SEEK_SET);
        return NULL;
    }

    struct disk_file_header *h = (struct disk_file_header *) 
            calloc(1, sizeof(struct disk_file_header));
    if (h == NULL)
        err(1, "calloc error in read_disk_file_header().");

    fr = fread(&(h->compression_method), sizeof(uint8_t), 1, fp);
    check_file_read(file_name, fp, 1, fr);

    fr = fread(&(h->flag), sizeof(uint8_t), 1, fp);
    check_file_read(file_name, fp, 1, fr);

    fr = fread(&(h->extra), sizeof(uint8_t), 1, fp);
    check_file_read(file_name, fp, 1, fr);
    
    return h;
}
