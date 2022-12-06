HTS_ROOT=../../lib/htslib

metadata:
	gcc ../metadata_index.c ../query_filter.c metadata_test.c -I$(HTS_ROOT) $(HTS_ROOT)/libhts.a -o bin/metadata_test -g && bin/metadata_test

metadata-mem:
	valgrind --leak-check=full --show-leak-kinds=all bin/metadata_test
