BIN=bin
OBJ=obj

# Detect host system for configure
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_S),Darwin)
    ifeq ($(UNAME_M),arm64)
        HOST_FLAGS := --build=aarch64-apple-darwin --host=aarch64-apple-darwin
    else ifeq ($(UNAME_M),x86_64)
        HOST_FLAGS := --build=x86_64-apple-darwin --host=x86_64-apple-darwin
    endif
endif

all: htslib
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE)

htslib:
	$(shell cd lib/htslib && autoreconf)
	cd lib/htslib; ./configure --disable-bz2 --disable-lzma --enable-libcurl $(HOST_FLAGS)
	$(MAKE) -C lib/htslib

server:
	@mkdir -p $(OBJ)
	@mkdir -p $(BIN)
	cd src; $(MAKE) server

clean:
	rm -rf $(BIN)/*
	rm -rf $(OBJ)/*
	cd lib/htslib && $(MAKE) clean
