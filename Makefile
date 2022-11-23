BIN=bin
OBJ=obj

all: htslib
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE)

htslib:
	$(shell cd lib/htslib && autoreconf)
	cd lib/htslib; ./configure --disable-bz2 --disable-lzma --enable-libcurl
	$(MAKE) -C lib/htslib

server:
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE) server

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
	cd lib/htslib && $(MAKE) clean

HTS_ROOT=../../lib/htslib

metadata:
	cd src/metadata && gcc ../metadata_index.c ../query_filter.c metadata_test.c -I$(HTS_ROOT) $(HTS_ROOT)/libhts.a -o bin/metadata_test -g && bin/metadata_test

metadata-mem:
	cd src/metadata && valgrind --leak-check=full --show-leak-kinds=all bin/metadata_test
