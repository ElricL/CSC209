# Makefile for programs to index and search an index.

FLAGS = -Wall -g -std=gnu99
SRC = freq_list.c punc.c
HDR = freq_list.h worker.h
OBJ = freq_list.o punc.o

define result1
2    menu1
6    menu2
endef

define newline


endef

all : indexer queryone printindex query

indexer : indexer.o ${OBJ}
	gcc ${FLAGS} -o $@ indexer.o ${OBJ}

printindex : printindex.o ${OBJ}
	gcc ${FLAGS} -o $@ printindex.o ${OBJ}

queryone : queryone.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ queryone.o worker.o ${OBJ}

query : query.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ query.o worker.o ${OBJ}

test: testfile.o worker.o ${OBJ}
	gcc ${FLAGS} -o $@ testfile.o worker.o ${OBJ}

# Test a word that does exist in the word linkedlist
test_part1: test
	@test_part1_output=`./test pepper`;\
	if [ ! -z "$$test_part1_output" ] && [ "$$test_part1_output" = "1    menu2" ]; then \
		echo Compiled and sanity check passed; \
	else \
		echo Failed sanity check; \
	fi

# Test a word that doesn't exist in the word linkedlist
test_part2: test
	@test_part2_output=`./test none`;\
	if [ ! -z "$$test_part2_output" ] && [ "$$test_part2_output" = "Invalid word" ]; then \
		echo Compiled and sanity check passed; \
	else \
		echo Failed sanity check; \
	fi

# test output with multiple lines
test_part3: test
	@test_part3_output=`./test spinach > output.txt`;\
	    diff -s output.txt expected.txt; \

# Separately compile each C file
%.o : %.c ${HDR}
	gcc ${FLAGS} -c $<

clean :
	-rm *.o indexer queryone printindex


