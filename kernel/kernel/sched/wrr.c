#include "sched.h"
#include "linux/init_task.h"
/*
 * for get_cpu()
 */
#include "linux/smp.h"
#include <linux/kernel.h>

const struct sched_class wrr_sched_class;

static inline struct task_struct *wrr_task_of(struct sched_wrr_entity *wrr)
{
	return container_of(wrr, struct task_struct, wrr);
}

static void
enqueue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se, bool head)
{
	struct list_head *queue = &rq->wrr.wrr_task_list;

	if (head)
		list_add(&wrr_se->wrr_task_list, queue);
	else
		list_add_tail(&wrr_se->wrr_task_list, queue);
	++rq->wrr.wrr_nr_running;
        atomic_add(wrr_se->wrr_weight, &rq->wrr.total_weight);
}

static void
dequeue_wrr_entity(struct rq *rq, struct sched_wrr_entity *wrr_se)
{
        list_del_init(&wrr_se->wrr_task_list);
        atomic_sub(wrr_se->wrr_weight, &rq->wrr.total_weight);
        --rq->wrr.wrr_nr_running;
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
        wrr_rq->wrr_nr_running = 0;
}

static void watchdog(struct rq *rq, struct task_struct *p)
{
	unsigned long soft, hard;

	/* max may change after cur was read, this will be fixed next tick */
	soft = task_rlimit(p, RLIMIT_RTTIME);
	hard = task_rlimit_max(p, RLIMIT_RTTIME);

	if (soft != RLIM_INFINITY) {
		unsigned long next;

		p->wrr.timeout++;

		next = DIV_ROUND_UP(min(soft, hard), USEC_PER_SEC/HZ);
		if (p->wrr.timeout > next)
			p->cputime_expires.sched_exp = p->se.sum_exec_runtime;
	}
}

static void update_curr_wrr(struct rq *rq)
{
        struct task_struct *curr = rq->curr;
        u64 delta_exec;

        if (curr->sched_class != &wrr_sched_class)
                return;

        delta_exec = rq->clock_task - curr->se.exec_start;
        if (unlikely((s64)delta_exec <= 0))
                delta_exec = 0;

        schedstat_set(curr->se.statistics.exec_max,
                        max(curr->se.statistics.exec_max, delta_exec));

        curr->se.sum_exec_runtime += delta_exec;
        curr->se.exec_start = rq->clock_task;
        cpuacct_charge(curr, delta_exec);
}

static void
enqueue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        struct sched_wrr_entity *wrr_se = &p->wrr;

	if (flags & ENQUEUE_WAKEUP)
		wrr_se->timeout = 0;

	printk("Enqueuing\n");
        enqueue_wrr_entity(rq, wrr_se, flags & ENQUEUE_HEAD);
        add_nr_running(rq, 1);
}

static void dequeue_task_wrr(struct rq *rq, struct task_struct *p, int flags)
{
        struct sched_wrr_entity *wrr_se = &p->wrr;

        update_curr_wrr(rq);
	printk("Dequeuing\n");
        dequeue_wrr_entity(rq, wrr_se);
        sub_nr_running(rq, 1);
}

static void requeue_task_wrr(struct rq *rq, struct task_struct *p)
{
	list_move_tail(&p->wrr.wrr_task_list, &rq->wrr.wrr_task_list);
}

static void yield_task_wrr(struct rq *rq)
{
        requeue_task_wrr(rq, rq->curr);
}

/*No preemption involved*/
static void check_preempt_curr_wrr(struct rq *rq, struct task_struct *p, int flags)
{
}

static struct task_struct *
pick_next_task_wrr(struct rq *rq, struct task_struct *prev)
{
        struct task_struct *next;
        struct sched_wrr_entity *next_ent;

        if (!rq->wrr.wrr_nr_running)
                return NULL;

	printk("Picking\n");
        next_ent = list_first_entry(&rq->wrr.wrr_task_list,
        		struct sched_wrr_entity, wrr_task_list);
        next = wrr_task_of(next_ent);
	if (!next)
		return NULL;

	next->se.exec_start = rq->clock;
        return next;
}

static void put_prev_task_wrr(struct rq *rq, struct task_struct *prev)
{
        update_curr_wrr(rq);
        prev->se.exec_start = 0;
}

#ifdef CONFIG_SMP
static int
select_task_rq_wrr(struct task_struct *p, int cpu, int sd_flag, int flags)
{
	int i;
	int min_cpu;
	int min_cpu_weight;
	int this_cpu_weight;

	rcu_read_lock();

	/*
	* get this cpu's total weight and suppose it is min
	*/
	min_cpu_weight = atomic_read(&this_rq()->wrr.total_weight);
	min_cpu = get_cpu();

	for_each_online_cpu(i) {
		struct wrr_rq *wrr_rq = &cpu_rq(cpu)->wrr;
		this_cpu_weight = atomic_read(&wrr_rq->total_weight);
		if (this_cpu_weight < min_cpu_weight) {
		    min_cpu_weight = this_cpu_weight;
		    min_cpu = i;
		}
	}

	rcu_read_unlock();

	return min_cpu;
}
#endif

static void set_curr_task_wrr(struct rq *rq)
{
	struct task_struct *p = rq->curr;

	p->se.exec_start = rq->clock_task;
}

static void task_tick_wrr(struct rq *rq, struct task_struct *p, int queued)
{
	struct sched_wrr_entity *wrr_se = &p->wrr;

	update_curr_wrr(rq);

	watchdog(rq, p);

	if (p->policy != SCHED_WRR)
		return;

	if (--wrr_se->time_slice)
		return;

	wrr_se->time_slice = wrr_se->wrr_weight * BASE_WRR_TIMESLICE;

	/*
	 * Requeue to the end of queue if we (and all of our ancestors) are not
	 * the only element on the queue
	 */
	if (wrr_se->wrr_task_list.prev != wrr_se->wrr_task_list.next) {
		requeue_task_wrr(rq, p);
		resched_curr(rq);
		return;
	}
}

/*No prio involved*/
static void
prio_changed_wrr(struct rq *rq, struct task_struct *p, int oldprio)
{
}

static void switched_to_wrr(struct rq *rq, struct task_struct *p)
{
        if (task_on_rq_queued(p) && rq->curr != p) {
		if (p->prio < rq->curr->prio && cpu_online(cpu_of(rq)))
			resched_curr(rq);
	}
}


static unsigned int get_rr_interval_wrr(struct rq *rq, struct task_struct *task)
{
	return task->wrr.wrr_weight * BASE_WRR_TIMESLICE;
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

        .get_rr_interval	= get_rr_interval_wrr,

	.prio_changed		= prio_changed_wrr,
	.switched_to		= switched_to_wrr,

	.update_curr		= update_curr_wrr,
};

/*
 * helper to check if the task can be remove from one cpu
 */
static int
can_migrate_task(struct task_struct *p, struct rq *src_rq, struct rq *tst_rq)
{
	if (!cpumask_test_cpu(tst_rq->cpu, tsk_cpus_allowed(p)))
		return 0;
	if (!cpu_online(tst_rq->cpu))
		return 0;
	if (task_cpu(p) != src_rq->cpu)
		return 0;
	if (task_running(src_rq, p))
		return 0;
	return 1;
}

static int idle_balance(struct rq *this_rq)
{

	int this_cpu = this_rq->cpu;
	int temp_cpu;
	int max_weight_cpu = this_rq->cpu;
	int max_weight = 0;
	int last_idle_cpu;
	int idle_cpu_number = 0;
	int cpu_number = 0;

	struct task_struct *temp_task_struct;
	struct sched_wrr_entity *wrr_se;

	rcu_read_lock();

	/*
	 * idle_cpu is from core.c
	 */
	for_each_online_cpu(temp_cpu) {
		struct rq *temp_rq = cpu_rq(temp_cpu);
		struct wrr_rq *wrr_rq = &temp_rq->wrr;

		/*
		 * get a idle cpu
		 */
		if (idle_cpu(temp_cpu)) {
			last_idle_cpu = temp_cpu;
			idle_cpu_number ++;
		}

		/*
		 * get the max weight cpu
		 */
		if (max_weight < wrr_rq->total_weight) {
			max_weight = wrr_rq->total_weight;
			max_weight_cpu = temp_cpu;
		}

		cpu_number++;
	}

	/*
	 * some check constrains
	 */
	struct rq *max_rq = cpu_rq(max_weight_cpu);
	struct rq *idle_rq = cpu_rq(last_idle_cpu);
	struct wrr_rq *max_wrr_rq = &max_rq->wrr;

	if (idle_cpu_number == 0)
		return;

	if (cpu_number < 2)
		return;

	if (max_wrr_rq->wrr_nr_running < 2)
		return;

	list_for_each_entry(wrr_se, max_wrr_rq->wrr_task_list) {
		temp_task_struct = wrr_task_of(wrr_se);
		if (!can_migrate_task()) {
			continue;
		}

		/*
		 * move_queued_task is from core.c
		 */
		move_queued_task(max_rq, temp_task_struct, last_idle_cpu);
		break;
	}
}
