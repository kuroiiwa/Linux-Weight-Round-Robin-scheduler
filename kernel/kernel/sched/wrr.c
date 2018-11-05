#include "sched.h"

/*
 * for get_cpu()
 */
#include "linux/smp.h"

unsigned int sched_wrr_time_slice = 10000UL;

static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr)
{
	return container_of(wrr, struct task_struct, wrr);
}

static void
enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
	struct list_head *queue = &rq->wrr_rq.wrr_task_list;

	if (head)
		list_add(&wrr_se->wrr_task_list, queue);
	else
		list_add_tail(&wrr_se->wrr_task_list, queue);
	++rq->wrr.wrr_nr_running;
        atomic_add(&wrr_se.wrr_weight, &rq->wrr_rq.total_weight);
}

static struct sched_wrr_entity *
pick_next_wrr_entity(struct wrr_rq *wrr)
{
        struct sched_wrr_entity *next = NULL;

        next = list_entry(wrr->wrr_task_list.next, struct sched_wrr_entity,
                        wrr_task_list);
        return next;
}

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
        atomic_set(&wrr_rq->total_weight, 0);
        INIT_LIST_HEAD(&wrr_rq->wrr_task_list);
}

static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        struct sched_wrr_entity *wrr_se = &p->wrr;

	if (flags & ENQUEUE_WAKEUP)
		wrr_se->timeout = 0;

	enqueue_wrr_entity(wrr_se, flags & ENQUEUE_HEAD);
        add_nr_running(rq, 1);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        list_del_init(&p->wrr.wrr_task_list);
        atomic_dec(p->wrr.wrr_weight, &rq->wrr_rq.total_weight);
        --rq->wrr.rt_nr_running;
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p)
{
	list_move_tail(&p->wrr.wrr_task_list, &rq->wrr_rq.wrr_task_list);
}

static void yield_task_wrr(struct rq *rq)
{
        requeue_task_wrr(rq, rq->curr);
}

static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev)
{
        struct task_struct *next = NULL;
        struct sched_wrr_entity *next_ent = NULL;

        if (list_empty(&rq->wrr.wrr_task_list))
                return next;
        next_ent = pick_next_wrr_entity(&rq->wrr);
        if (!next_ent)
                return -EFAULT;
        next = wrr_task_of(next_ent);

        return next;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
}

#ifdef CONFIG_SMP
static int
select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
    int cpu;
    int min_cpu;
    int min_cpu_weight;
    int this_cpu_weight;

    rcu_read_lock();

    /*
     * get this cpu's total weight and suppose it is min
     */
    min_cpu_weight = this_rq()->wrr->total_weight;
    min_cpu = get_cpu();

    for_each_online_cpu(cpu) {
        struct wrr_rq *wrr_rq = &cpu_rq(cpu)->wrr;
        this_cpu_weight = wrr_rq->total_weight;
        if (this_cpu_weight < min_cpu_weight) {
            min_cpu_weight = this_cpu_weight;
            min_cpu = cpu;
        }
    }

    rcu_read_unlock();

    return min_cpu;
}
#endif

static void set_curr_task_wrr(struct rq *rq)
{
}

static void task_tick_wrr(struct rq *rq, struct task_struct *curr, int queued)
{
}

static void
prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
}

static void update_curr_wrr(struct rq *rq)
{
        struct task_struct *curr = rq-curr;
        u64 delta_exec;

        if not policy
        delta_exec = rq->clock - curr->wrr.exec_start;
        if delta_exec < 0


}


const struct sched_class wrr_sched_class = {
	.next			= &fair_sched_class,
	.enqueue_task		= enqueue_task_wrr,
	.dequeue_task		= dequeue_task_wrr,
        .yield_task		= yield_task_wrr,

	.check_preempt_curr	= check_preempt_curr_wrr,

	.pick_next_task		= pick_next_task_wrr,
	.put_prev_task		= put_prev_task_wrr,

#ifdef CONFIG_SMP
	.select_task_rq		= select_task_rq_wrr,
	.set_cpus_allowed       = set_cpus_allowed_common,
#endif

	.set_curr_task          = set_curr_task_wrr,
	.task_tick		= task_tick_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,

	.update_curr		= update_curr_wrr,
};
