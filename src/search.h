#ifndef __SEARCH_H__
#define __SEARCH_H__

#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <htslib/kstring.h>
#include <math.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "cache.h"
#include "file_read.h"
#include "giggle_index.h"
#include "kfunc.h"
#include "ll.h"
#include "search.h"
#include "util.h"
#include "wah.h"



#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

int search_help(int exit_code);

int print_giggle_query_result(struct giggle_query_result *gqr,
                              struct giggle_index *gi, regex_t *regexs,
                              char **file_patterns, uint32_t num_file_patterns,
                              uint32_t num_intervals, double mean_interval_size,
                              long long genome_size, uint32_t f_is_set,
                              uint32_t v_is_set, uint32_t c_is_set,
                              uint32_t s_is_set, uint32_t o_is_set);

int search_main(int argc, char **argv, char *full_cmd);

#endif
