#include "index.h"
#include "cache.h"
#include "giggle_index.h"
#include "ll.h"
#include "util.h"
//#include "wah.h"
#include <ctype.h>
#include <err.h>
#include <getopt.h>
#include <inttypes.h>
//#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

// int index_help(int exit_code);
// int index_main(int argc, char **argv, char *full_cmd);

int index_help(int exit_code) {
  fprintf(stderr,
          "%s, v%s\n"
          "usage:   %s index -i <input files> -o <output dir> -f\n"
          "         options:\n"
          "             -s  Files are sorted\n"
          "             -i  Files to index (e.g. data/*.gz)\n"
          "             -o  Index output directory\n"
          "             -m  Metadata config file\n"
          "             -f  For reindex if output directory exists\n",
          PROGRAM_NAME, VERSION, PROGRAM_NAME);
  return exit_code;
}

int index_main(int argc, char **argv, char *full_cmd) {
  if (argc < 2)
    return index_help(EX_OK);

  uint32_t num_chrms = 100;
  int c;
  char *input_pattern = NULL, *output_dir_name = NULL,
       *metadata_conf_name = NULL;
  char *i_type = "i";

  int i_is_set = 0, o_is_set = 0, s_is_set = 0, f_is_set = 0, m_is_set = 0;

  while ((c = getopt(argc, argv, "i:o:m:fsh")) != -1) {
    switch (c) {
    case 'i':
      i_is_set = 1;
      input_pattern = optarg;
      break;
    case 'o':
      o_is_set = 1;
      output_dir_name = optarg;
      break;
    case 'm':
      m_is_set = 1;
      metadata_conf_name = optarg;
      break;
    case 'f':
      f_is_set = 1;
      break;
    case 's':
      s_is_set = 1;
      break;
    case 'h':
      return index_help(EX_OK);
    case '?':
      if ((optopt == 'i') || (optopt == 'o') || (optopt == 'm'))
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      return index_help(EX_USAGE);
    default:
      return index_help(EX_OK);
    }
  }

  if (i_is_set == 0) {
    fprintf(stderr, "Input file is not set\n");
    return index_help(EX_USAGE);
  } else if (o_is_set == 0) {
    fprintf(stderr, "Output file is not set\n");
    return index_help(EX_USAGE);
  }

  /* Handling the giggle input and output directory paths:
   *
   * Assumptions:
   * 1. giggle input_dir_name is a glob pattern that is only in a single
   * directory
   * 2. giggle output_dir_name and input_dir_name have the same parent
   *     - we could technically let the input_dir_name glob pattern cover
   * several directories as long as their parent is the same as the
   * output_dir_name, but for now it's easiest to just require the
   * input_dir_name to be a single directory.
   *
   * Procedure:
   * 1. Get the absolute path for one of the input files (using glob)
   * 2. Get the parent directory (basename of dirname)
   * 3. chdir to the parent directory so that everything is relative to that
   * directory
   * 4. Pass the $data_dir/$glob_pattern to giggle indexing functions where
   * $data_dir is the basename(dirname) of input_dir_name and $glob_pattern is
   * basename of input_dir_name eg. if input_dir_name is "/path/to/data/\*.gz",
   * then data_dir = "data".
   */

  // Get the absolute path for one of the input files (using glob)
  char glob_abs_path[4096];
  abs_path_of_glob(input_pattern, glob_abs_path);

  char glob_basename[4096];
  safe_basename(input_pattern, glob_basename);

  // input data is directly under this directory
  char input_dirname[4096];
  safe_dirname(glob_abs_path, input_dirname);

  // working directory for building the index
  char parent_dirname[4096];
  safe_dirname(input_dirname, parent_dirname);

  char output_parent_dirname[4096];
  safe_dirname(output_dir_name, output_parent_dirname);
  char output_parent_abs_path[4096];
  if (realpath(output_parent_dirname, output_parent_abs_path) == NULL) {
    fprintf(stderr, "Invalid output directory path: %s\n",
            output_parent_dirname);
    exit(EXIT_FAILURE);
  }

  if (strcmp(parent_dirname, output_parent_abs_path) != 0) {
    fprintf(stderr,
            "Input and output directories must have the same parent.\n");
    exit(EXIT_FAILURE);
  }

  // just the directory names of the input/output data without the rest of the
  // path
  char input_dir_only[4096];
  char output_dir_only[4096];
  safe_basename(input_dirname, input_dir_only);
  safe_basename(output_dir_name, output_dir_only);

  char input_dir_with_glob[4096];
  snprintf(input_dir_with_glob, 4096, "%s/%s", input_dir_only, glob_basename);

  chdir(parent_dirname);
  struct giggle_index *gi;

  if (s_is_set == 1) {
    uint64_t num_intervals = giggle_bulk_insert_with_metadata(
        input_dir_with_glob, output_dir_only, metadata_conf_name, f_is_set);
    fprintf(stderr, "Indexed %" PRIu64 " intervals.\n", num_intervals);
  } else {
    gi = giggle_init_with_metadata(num_chrms, output_dir_only,
                                   metadata_conf_name, f_is_set,
                                   uint64_t_ll_giggle_set_data_handler);
    if (gi == NULL)
      return EX_DATAERR;

    uint32_t r = giggle_index_directory(gi, input_dir_with_glob, 0);

    fprintf(stderr, "Indexed %u intervals.\n", r);

#if BLOCK_STORE
    giggle_data_handler.write_tree = giggle_write_tree_leaf_data;
#endif

    r = giggle_store(gi);

    if (r != 0)
      errx(1, "Error storing giggle index.");

    giggle_index_destroy(&gi);
    cache.destroy();
  }
  return EX_OK;
}
