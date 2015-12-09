UNITY_ROOT=./unity
HTS_ROOT=$(HOME)/src/htslib
CC=gcc
SRC=../../src/
TEST_SRC=$(wildcard *.c)
TEST_NAMES=$(TEST_SRC:.c=)
PREFIX=Runner_
RUNNERS=$(addprefix $(PREFIX),$(TESTS))

NO_PRE_SRC_FILES=giggle.c \
		 bpt.c \
		 ll.c \
		 file_read.c \
		 lists.c 

SRC_FILES=$(addprefix $(SRC),$(NO_PRE_SRC_FILES))

all: clean $(TEST_NAMES) test

%: %.c
	#ruby $(UNITY_ROOT)/auto/generate_test_runner.rb $@.c
	ruby $(UNITY_ROOT)/generate_test_runner.rb $@.c
	$(CC) -Werror -g -o $@ \
	    -lz \
	    -I$(UNITY_ROOT) \
	    -I$(SRC) \
	    -I$(HTS_ROOT) \
	    -DTEST $(UNITY_ROOT)/unity.c \
	    -DSAMTOOLS=1 \
	    $(HTS_ROOT)/libhts.a \
	    -pthread \
	    $(SRC_FILES) \
	    -ll \
	    $@.c $@_Runner.c -lm 
	rm $@_Runner.c

test:
	$(foreach var,$(TEST_NAMES),./$(var);)

clean:
	rm -f $(TEST_NAMES)
	rm -f *_Runner.c
