#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/* calculate bin given an alignment covering [beg,end) (zero-based,
 * half-closed-half-open) */
int reg2bin(int beg, int end)
{
    --end;
    if (beg>>14 == end>>14) return ((1<<15)-1)/7 + (beg>>14);
    if (beg>>17 == end>>17) return ((1<<12)-1)/7 + (beg>>17);
    if (beg>>20 == end>>20) return ((1<<9)-1)/7 + (beg>>20);
    if (beg>>23 == end>>23) return ((1<<6)-1)/7 + (beg>>23);
    if (beg>>26 == end>>26) return ((1<<3)-1)/7 + (beg>>26);
    return 0;
}

int main(int argc, char **argv)
{
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if (argc != 3)
        errx(1,"usage:\t%s <file_id> <file_name> \n", argv[0]);

    int file_id = atoi(argv[1]);
    char *file_name = argv[2];

    fp = fopen(file_name, "r");
    if (fp == NULL)
        err(1, "Could not open file '%s'", file_name);

    while ((read = getline(&line, &len, fp)) != -1) {
        char *chrm = strtok(line, "\t");
        int beg = atoi(strtok(NULL, "\t"));
        int end = atoi(strtok(NULL, "\t"));
        printf("INSERT INTO INTERVALS VALUES(%d,%d,\"%s\",%d,%d);",
               file_id,
               reg2bin(beg,end),
               chrm,
               beg,
               end);
    }

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}

