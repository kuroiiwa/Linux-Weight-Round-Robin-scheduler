#include "sched.h"

unsigned int sched_wrr_time_slice = 10000UL;

void init_wrr_rq(struct wrr_rq *wrr_rq)
{
        atomic_set(&wrr_rq->total_weight, 0);
        INIT_LIST_HEAD(&wrr_rq->wrr_task_list);
}

static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        list_add_tail(&p->wrr.wrr_task_list, &rq->wrr_rq.wrr_task_list);
        atomic_add(p->wrr.wrr_weight, &rq->wrr_rq.total_weight);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        list_del(&p->wrr.wrr_task_list);
        atomic_dec(p->wrr.wrr_weight, &rq->wrr_rq.total_weight);
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

}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
}

static int
select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
}

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
