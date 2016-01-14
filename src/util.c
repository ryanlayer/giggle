#include <err.h>
#include <sysexits.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

void check_file_read(char *file_name, FILE *fp, size_t exp, size_t obs)
{
    if (exp != obs) {
        if (feof(fp))
            errx(EX_IOERR,
                 "Error reading file \"%s\": End of file",
                 file_name);
        err(EX_IOERR, "Error reading file \"%s\"", file_name);
    }
}
