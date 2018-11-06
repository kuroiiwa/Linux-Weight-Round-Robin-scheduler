#ifndef _LINUX_ORIENTATION_H
#define _LINUX_ORIENTATION_H
#define MAX_CPUS 6
struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};

#endif
