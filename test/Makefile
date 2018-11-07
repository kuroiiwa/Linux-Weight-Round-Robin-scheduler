CC := gcc

########## YOUR MAKEFILE BELOW ##########
CFLAGS := -g -O2 -Wall -Werror

all: test gen_proc

test: test.c
	$(CC) $(CFLAGS) -o test test.c 

gen_proc: gen_proc.c
	$(CC) $(CFLAGS) -o gen_proc gen_proc.c 

clean:
	rm -f test gen_proc
.PHONY: clean
