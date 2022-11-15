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

mdt1:
	cd src/metadata && gcc metadata_index.c query_filter.c metadata_test1.c -I$(HTS_ROOT) $(HTS_ROOT)/libhts.a -o bin/metadata_test1 -g && bin/metadata_test1

mdt1-mem:
	cd src/metadata && valgrind --leak-check=full --show-leak-kinds=all bin/metadata_test1
