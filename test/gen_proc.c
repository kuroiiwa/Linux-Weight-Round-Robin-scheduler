#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define __NR_setscheduler 144
#define MAX_CPUS 6 /* We will be testing only on the VMs */
#define __NR_get_wrr_info 326
#define __NR_set_wrr_weight 327
#define CHILD_PROC 6


struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

struct sched_param {
	int sched_priority;
};

void call(void) 
{
	struct wrr_info info;
	int res;

	res = syscall(__NR_get_wrr_info, &info);
	if (!res) {
		printf("num_cpus: %d\n", info.num_cpus);
		for (int i = 0; i < info.num_cpus; i++) {
			printf("CPU_%d: %d %d\n", i, info.nr_running[i],
					info.total_weight[i]);
		}
	}
}
int main(int argc, char **argv)
{
	pid_t pid;
	int weight;
	int pids[CHILD_PROC];
	int n = 0;
	int i = 0;

	for (int i = 0; i < CHILD_PROC; i++) {
		pid = fork();
		if (pid < 0)
			return -1;

		if (pid == 0) {
			while (1) {
				if (n == 50000000) {
					printf("CPU BOUND Process%d\n", pid);
					i++;
					n = 0;
				}
				n++;
				if (i == 10000) 
					exit(0);
			}
		} else
			pids[i] = pid;
	}
	call();
	usleep(10000000);


	return 0;
}
