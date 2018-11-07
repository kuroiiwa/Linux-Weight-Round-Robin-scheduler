CC := gcc

########## YOUR MAKEFILE BELOW ##########
CFLAGS := -g -O2 -Wall -Werror
EXE := test1 test_set_weight call_wrr_info gen_proc

all: test_set_weight call_wrr_info gen_proc

test_set_weight: test_set_weight.c
	$(CC) $(CFLAGS) -o test_set_weight test_set_weight.c

call_wrr_info: call_wrr_info.c
	$(CC) $(CFLAGS) -o call_wrr_info call_wrr_info.c

gen_proc: gen_proc.c
	$(CC) $(CFLAGS) -o gen_proc gen_proc.c

clean:
	rm -f $(EXE)
