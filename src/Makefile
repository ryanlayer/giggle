BIN=../bin
OBJ=../obj
HTS_ROOT=../../htslib
LIBMHD_INCLUDES=$(HOME)/usr/local/include
LIBMHD_LIBS=$(HOME)/usr/local/lib
CFLAGS=-O2 -D_FILE_OFFSET_BITS=64 -Werror -Wuninitialized
INCLUDES=-I$(HTS_ROOT)
LIBS=-ldl -lz -lm -pthread 
#CFLAGS=-g -D_FILE_OFFSET_BITS=64 -Werror -Wuninitialized
#CFLAGS=-Wall -O0 -g -fno-inline -fkeep-inline-functions -D_FILE_OFFSET_BITS=64 -fPIC -DDEBUG -D_DEBUG
CC=gcc


LIB=bpt.o \
    disk_store.o \
    cache.o \
    data_reg.o \
    file_read.o \
    giggle_index.o \
    lists.o \
    ll.o \
    util.o \
    timer.o \
    wah.o \
    kfunc.o \
    index.o \
    search.o


.SUFFIXES:

OBJS=$(addprefix $(OBJ)/, $(LIB))

.SECONDARY: $(OBJS)


PROG=index_search \
     search_file \
     test \
     sig_test \
     speed_tests \
     api_test \
     giggle

LIST=$(addprefix $(BIN)/, $(PROG))


all: check-env $(LIST)

server: INCLUDES+=-I$(LIBMHD_INCLUDES) -L$(LIBMHD_LIBS)
server: LIBS+=-lmicrohttpd
server: $(BIN)/server_overlap

$(OBJ)/%.o: %.c 
	$(CC) -c $(CFLAGS) -o $@ $< \
		-I$(HTS_ROOT)


$(BIN)/%: %.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ \
		$(INCLUDES) \
		-DSAMTOOLS=1 \
		$(HTS_ROOT)/libhts.a \
		$(LIBS)


check-env:
ifdef TRAVIS_BUILD_DIR
HTS_ROOT=$(TRAVIS_BUILD_DIR)/htslib
endif


