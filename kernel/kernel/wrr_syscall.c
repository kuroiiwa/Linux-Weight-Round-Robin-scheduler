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

struct wrr_info;

SYSCALL_DEFINE1(get_wrr_info, struct wrr_info __user *, info)
{
        struct task_struct *p;
        struct sched_wrr_entity *wrr_se;
        int i;

        p = current;
        wrr_se = &p->wrr;
        printk("Policy: %d\n", p->policy);
        printk("wrr info: %u, %u\n", wrr_se->wrr_weight, wrr_se->time_slice);
        if (p->sched_class == &wrr_sched_class)
                printk("In the class!\n");
        if (p->sched_class == &fair_sched_class)
                printk("In CFS\n");
        if (p->sched_class == &rt_sched_class)
                printk("In RT\n");

        for_each_online_cpu(i) {
            struct wrr_rq *wrr_rq = &cpu_rq(i)->wrr;
            printk("weight: %d\n", atomic_read(&wrr_rq->total_weight));
            printk("running: %u\n", wrr_rq->wrr_nr_running);
        }
        return 1;
}
