BIN=../bin
OBJ=../obj
#BPT_ROOT=$(HOME)/src/cs/b_plus_tree
HTS_ROOT=../../htslib
CFLAGS=-O2 -D_FILE_OFFSET_BITS=64 -Werror -Wuninitialized
#CFLAGS=-Wall -O0 -g -fno-inline -fkeep-inline-functions -D_FILE_OFFSET_BITS=64 -fPIC -DDEBUG -D_DEBUG
CC=gcc

LIB=bpt.o

.SUFFIXES:

OBJS=$(addprefix $(OBJ)/, $(LIB))

.SECONDARY: $(OBJS)


PROG=giggle


LIST=$(addprefix $(BIN)/, $(PROG))

all: $(LIST)


$(OBJ)/%.o: %.c 
	$(CC) -c $(CFLAGS) -o $@ $< \
		-I$(HTS_ROOT)

$(BIN)/%: %.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ \
		-I$(HTS_ROOT) \
		-DSAMTOOLS=1 \
		$(HTS_ROOT)/libhts.a \
		-ldl -lz -lm  -pthread
