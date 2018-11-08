#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_CPUS 6 /* We will be testing only on the VMs */
#define __NR_get_wrr_info 326
#define __NR_set_wrr_weight 327
#define __NR_setscheduler 144
#define N 1874919423


struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};
struct sched_param {
	int sched_priority;
};

void find_factors(int n, int io)
{
	for (int i = 1; i <= n; ++i) {
		if (n % i == 0 && io)
			printf("%d", i);
	}
}

int main(int argc, char **argv)
{
	struct wrr_info info;
	struct sched_param param;
	pid_t pid;
	int res;
	int weight = atoi(argv[1]);

	param.sched_priority = 0;

	syscall(__NR_setscheduler, pid, 7, &param);
	syscall(__NR_set_wrr_weight, weight);
	find_factors(N, 0);

	return 0;
}
