UNITY_ROOT=./unity
HTS_ROOT=../../lib/htslib/
CC=gcc
SRC=../../src/
TEST_SRC = $(filter-out $(wildcard *_Runner.c), $(wildcard *.c))
TEST_NAMES=$(TEST_SRC:.c=)
PREFIX=Runner_
RUNNERS=$(addprefix $(PREFIX),$(TESTS))

NO_PRE_SRC_FILES= disk_store.c \
		 disk_file_header.c \
		 zlib_wrapper.c \
		 cache.c \
		 giggle_index.c \
		 lists.c \
		 util.c \
		 ll.c \
		 timer.c \
		 file_read.c \
		 bpt.c \
		 wah.c \
		 leaf.c \
		 jsw_avltree.c \
		 pq.c \
		 offset_index.c \
		 fastlz.c


SRC_FILES=$(addprefix $(SRC),$(NO_PRE_SRC_FILES))

all: check-env clean $(TEST_NAMES) test

check-env:
ifdef TRAVIS_BUILD_DIR
HTS_ROOT=$(TRAVIS_BUILD_DIR)/htslib
endif



%: %.c
	#ruby $(UNITY_ROOT)/auto/generate_test_runner.rb $@.c
	ruby $(UNITY_ROOT)/generate_test_runner.rb $@.c
	$(CC) -Werror -g -o $@.o \
	    -I$(UNITY_ROOT) \
	    -I$(SRC) \
	    -I$(HTS_ROOT) \
	    -DTEST $(UNITY_ROOT)/unity.c \
	    -DSAMTOOLS=1 \
	    -pthread \
	    $(SRC_FILES) \
	    $@.c $@_Runner.c \
	    $(HTS_ROOT)/libhts.a \
	    -lz -lm -lcurl -lcrypto -lssl
	#rm $@_Runner.c

test:
	$(foreach var,$(TEST_NAMES),./$(var).o;)

clean:
	rm -f *.o
	rm -f *_Runner.c
	rm -rf *dSYM/
