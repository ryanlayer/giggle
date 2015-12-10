#include <stdlib.h>
#include <stdio.h>

#include "giggle.h"

int main(int argc, char **argv)
{
    repair = uint32_t_ll_leading_repair;
    new_non_leading = uint32_t_ll_new_non_leading;
    new_leading = uint32_t_ll_new_leading;
    non_leading_SA_add_scalar = uint32_t_ll_non_leading_SA_add_scalar;
    non_leading_SE_add_scalar = uint32_t_ll_non_leading_SE_add_scalar;
    leading_B_add_scalar = uint32_t_ll_leading_B_add_scalar;
    leading_union_with_B = uint32_t_ll_leading_union_with_B;
    non_leading_union_with_SA = uint32_t_ll_non_leading_union_with_SA;
    non_leading_union_with_SA_subtract_SE = 
            uint32_t_ll_non_leading_union_with_SA_subtract_SE;

    //char *path_name = "../data/many/*bed.gz";
    char *path_name = argv[1];

    ORDER = 10;

    struct giggle_index *gi = giggle_init_index(23);
    
    uint32_t r = giggle_index_directory(gi, path_name);
    giggle_query_region(gi, argv[2], atoi(argv[3]), atoi(argv[4]));
}
