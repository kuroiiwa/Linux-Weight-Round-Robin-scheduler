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


struct wrr_info {
	int num_cpus;
	int nr_running[MAX_CPUS];
	int total_weight[MAX_CPUS];
};
struct sched_param {
	int sched_priority;
};

int main(int argc, char **argv)
{
	struct wrr_info info;
	struct sched_param param;
	int res;
	pid_t pid, pid_temp;
	int status;

	pid = getpid();
	param.sched_priority = 0;
	syscall(__NR_setscheduler, pid, 7, &param);
	res = syscall(__NR_get_wrr_info, &info);
	printf("create parent \n") ;
        if (!res) {
                for (int i = 0; i < info.num_cpus; i++) {
                        printf("CPU_%d: %d %d\n", i, info.nr_running[i],
                                                info.total_weight[i]);
                }
        }

        printf("after fork\n");
        pid_temp = fork();
        if (pid_temp == 0) {
        
        	printf("fork \n");
        	res = syscall(__NR_get_wrr_info, &info);
		if (!res) {
		        for (int i = 0; i < info.num_cpus; i++) {
		                printf("CPU_%d: %d %d\n", i, info.nr_running[i],
		                                        info.total_weight[i]);
		        }
		}
		exit(0);
        }
        
        if (pid_temp > 0) {
        	wait(&status);
		res = syscall(__NR_get_wrr_info, &info);
		printf("exit children \n");
		if (!res) {
			for (int i = 0; i < info.num_cpus; i++) {
			        printf("CPU_%d: %d %d\n", i, info.nr_running[i],
			                                info.total_weight[i]);
			}

        }
        		}
	while(1);
        
	return 0;
}
