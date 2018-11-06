#include <linux/syscalls.h>
#include <linux/compiler.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include "linux/smp.h"
#include <../kernel/sched/sched.h>
#include <linux/wrr_syscall.h>

struct wrr_info;

SYSCALL_DEFINE1(get_wrr_info, struct wrr_info __user *, info)
{
        struct task_struct *p = current;
        struct wrr_info kinfo;
        int i;

        kinfo.num_cpus = 0;
        for_each_online_cpu(i) {
            struct wrr_rq *wrr_rq = &cpu_rq(i)->wrr;
            kinfo.nr_running[kinfo.num_cpus] = wrr_rq->wrr_nr_running;
            kinfo.total_weight[kinfo.num_cpus] =
                        atomic_read(&wrr_rq->total_weight);
            kinfo.num_cpus++;
        }
        if (copy_to_user(info, &kinfo, sizeof(struct wrr_info)))
		return -EFAULT;
        return 1;
}
