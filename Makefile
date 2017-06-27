BIN=bin
OBJ=obj

all: htslib
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE)

htslib:
	cd lib/htslib && \
	    	autoheader && autoconf && \
		./configure --disable-bz2 --disable-lzma --enable-libcurl && \
		$(MAKE)

server:
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE) server

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
	cd lib/htslib && $(MAKE) clean
