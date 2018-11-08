CC := gcc

########## YOUR MAKEFILE BELOW ##########
CFLAGS := -g -O2 -Wall -Werror
EXE := test1 test_set_weight call_wrr_info gen_cpu_proc gen_io_proc

all: test_set_weight call_wrr_info gen_cpu_proc gen_io_proc

test_set_weight: test_set_weight.c
	$(CC) $(CFLAGS) -o test_set_weight test_set_weight.c

call_wrr_info: call_wrr_info.c
	$(CC) $(CFLAGS) -o call_wrr_info call_wrr_info.c

gen_cpu_proc: gen_cpu_proc.c
	$(CC) $(CFLAGS) -o gen_cpu_proc gen_cpu_proc.c

gen_io_proc: gen_io_proc.c
	$(CC) $(CFLAGS) -o gen_io_proc gen_io_proc.c

clean:
	rm -f $(EXE)
