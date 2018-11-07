CC := gcc

########## YOUR MAKEFILE BELOW ##########
CFLAGS := -g -O2 -Wall -Werror

all: test1 test_set_weight
test1: test1.c
	$(CC) $(CFLAGS) -o test1 test1.c
	
test2: test_set_weight.c
	$(CC) $(CFLAGS) -o test_set_weight test_set_weight.c

.PHONY: clean

clean:
	rm -f test1 test2
