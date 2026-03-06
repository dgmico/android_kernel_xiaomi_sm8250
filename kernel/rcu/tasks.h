/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Task-based RCU implementations.
 *
 * Copyright (C) 2020 Paul E. McKenney
 */

#if defined(CONFIG_TASKS_RCU) || defined(CONFIG_TASKS_TRACE_RCU)

struct rcu_tasks;
typedef void (*rcu_tasks_gp_func_t)(struct rcu_tasks *rtp);
typedef void (*pregp_func_t)(void);
typedef void (*pertask_func_t)(struct task_struct *t, struct list_head *hop);
typedef void (*postscan_func_t)(struct list_head *hop);
typedef void (*holdouts_func_t)(struct list_head *hop, bool ndrpt, bool *frptp);
typedef void (*postgp_func_t)(struct rcu_tasks *rtp);

#define RTGS_INIT		 0
#define RTGS_WAIT_WAIT_CBS	 1
#define RTGS_WAIT_GP		 2
#define RTGS_PRE_WAIT_GP	 3
#define RTGS_SCAN_TASKLIST	 4
#define RTGS_POST_SCAN_TASKLIST	 5
#define RTGS_WAIT_SCAN_HOLDOUTS	 6
#define RTGS_SCAN_HOLDOUTS	 7
#define RTGS_POST_GP		 8
#define RTGS_WAIT_READERS	 9
#define RTGS_INVOKE_CBS		10
#define RTGS_WAIT_CBS		11

static atomic_t trc_n_readers_need_end;		// Number of waited-for readers.
static DECLARE_WAIT_QUEUE_HEAD(trc_wait);	// List of holdout tasks.

struct rcu_tasks {
	struct rcu_head *cbs_head;
	struct rcu_head **cbs_tail;
	struct wait_queue_head cbs_wq;
	raw_spinlock_t cbs_lock;
	int gp_state;
	int gp_sleep;
	int init_fract;
	unsigned long gp_jiffies;
	unsigned long gp_start;
	unsigned long n_gps;
	unsigned long n_ipis;
	unsigned long n_ipis_fails;
	struct task_struct *kthread_ptr;
	rcu_tasks_gp_func_t gp_func;
	pregp_func_t pregp_func;
	pertask_func_t pertask_func;
	postscan_func_t postscan_func;
	holdouts_func_t holdouts_func;
	postgp_func_t postgp_func;
	call_rcu_func_t call_func;
	char *name;
	char *kname;
};

/* Control stall timeouts.  Disable with <= 0, otherwise jiffies till stall. */
#define RCU_TASK_STALL_TIMEOUT (HZ * 60 * 10)
static int rcu_task_stall_timeout __read_mostly = RCU_TASK_STALL_TIMEOUT;
module_param(rcu_task_stall_timeout, int, 0644);

// Enqueue a callback for the specified flavor of Tasks RCU.
static void call_rcu_tasks_generic(struct rcu_head *rhp, rcu_callback_t func,
				   struct rcu_tasks *rtp)
{
	unsigned long flags;
	bool needwake;

	rhp->next = NULL;
	rhp->func = func;
	raw_spin_lock_irqsave(&rtp->cbs_lock, flags);
	needwake = !rtp->cbs_head;
	WRITE_ONCE(*rtp->cbs_tail, rhp);
	rtp->cbs_tail = &rhp->next;
	raw_spin_unlock_irqrestore(&rtp->cbs_lock, flags);
	/* We can't create the thread unless interrupts are enabled. */
	if (needwake && READ_ONCE(rtp->kthread_ptr))
		wake_up(&rtp->cbs_wq);
}

#define DEFINE_RCU_TASKS(rt_name, gp, call, n)				\
static struct rcu_tasks rt_name =					\
{									\
	.cbs_tail = &rt_name.cbs_head,					\
	.cbs_wq = __WAIT_QUEUE_HEAD_INITIALIZER(rt_name.cbs_wq),	\
	.cbs_lock = __RAW_SPIN_LOCK_UNLOCKED(rt_name.cbs_lock),		\
	.gp_func = gp,							\
	.call_func = call,						\
	.name = n,							\
	.kname = #rt_name,						\
}

/* Record grace-period phase and time. */
static void set_tasks_gp_state(struct rcu_tasks *rtp, int newstate)
{
	rtp->gp_state = newstate;
	rtp->gp_jiffies = jiffies;
}

static void rcu_tasks_wait_gp(struct rcu_tasks *rtp)
{
	struct task_struct *g, *t;
	unsigned long lastreport;
	LIST_HEAD(holdouts);
	int fract;

	set_tasks_gp_state(rtp, RTGS_PRE_WAIT_GP);
	rtp->pregp_func();

	/*
	 * There were callbacks, so we need to wait for an RCU-tasks
	 * grace period.  Start off by scanning the task list for tasks
	 * that are not already voluntarily blocked.  Mark these tasks
	 * and make a list of them in holdouts.
	 */
	set_tasks_gp_state(rtp, RTGS_SCAN_TASKLIST);
	rcu_read_lock();
	for_each_process_thread(g, t)
		rtp->pertask_func(t, &holdouts);
	rcu_read_unlock();

	set_tasks_gp_state(rtp, RTGS_POST_SCAN_TASKLIST);
	rtp->postscan_func(&holdouts);

	/*
	 * Each pass through the following loop scans the list of holdout
	 * tasks, removing any that are no longer holdouts.  When the list
	 * is empty, we are done.
	 */
	lastreport = jiffies;

	// Start off with initial wait and slowly back off to 1 HZ wait.
	fract = rtp->init_fract;
	if (fract > HZ)
		fract = HZ;

	for (;;) {
		bool firstreport;
		bool needreport;
		int rtst;

		if (list_empty(&holdouts))
			break;

		/* Slowly back off waiting for holdouts */
		set_tasks_gp_state(rtp, RTGS_WAIT_SCAN_HOLDOUTS);
		schedule_timeout_idle(HZ/fract);

		if (fract > 1)
			fract--;

		rtst = READ_ONCE(rcu_task_stall_timeout);
		needreport = rtst > 0 && time_after(jiffies, lastreport + rtst);
		if (needreport)
			lastreport = jiffies;
		firstreport = true;
		WARN_ON(signal_pending(current));
		set_tasks_gp_state(rtp, RTGS_SCAN_HOLDOUTS);
		rtp->holdouts_func(&holdouts, needreport, &firstreport);
	}

	set_tasks_gp_state(rtp, RTGS_POST_GP);
	rtp->postgp_func(rtp);
}

#endif /* #if defined(CONFIG_TASKS_RCU) || defined(CONFIG_TASKS_TRACE_RCU) */

#ifdef CONFIG_TASKS_RCU

/*
 * Simple variant of RCU whose quiescent states are voluntary context
 * switch, cond_resched_rcu_qs(), user-space execution, and idle.
 */

/* Global list of callbacks and associated lock. */
static struct rcu_head *rcu_tasks_cbs_head;
static struct rcu_head **rcu_tasks_cbs_tail = &rcu_tasks_cbs_head;
static DECLARE_WAIT_QUEUE_HEAD(rcu_tasks_cbs_wq);
static DEFINE_RAW_SPINLOCK(rcu_tasks_cbs_lock);

/* Track exiting tasks in order to allow them to be waited for. */
DEFINE_STATIC_SRCU(tasks_rcu_exit_srcu);

static struct task_struct *rcu_tasks_kthread_ptr;

/**
 * call_rcu_tasks() - Queue an RCU for invocation task-based grace period
 */
void call_rcu_tasks(struct rcu_head *rhp, rcu_callback_t func)
{
	unsigned long flags;
	bool needwake;

	rhp->next = NULL;
	rhp->func = func;
	raw_spin_lock_irqsave(&rcu_tasks_cbs_lock, flags);
	needwake = !rcu_tasks_cbs_head;
	*rcu_tasks_cbs_tail = rhp;
	rcu_tasks_cbs_tail = &rhp->next;
	raw_spin_unlock_irqrestore(&rcu_tasks_cbs_lock, flags);
	/* We can't create the thread unless interrupts are enabled. */
	if (needwake && READ_ONCE(rcu_tasks_kthread_ptr))
		wake_up(&rcu_tasks_cbs_wq);
}
EXPORT_SYMBOL_GPL(call_rcu_tasks);

/**
 * synchronize_rcu_tasks - wait until an rcu-tasks grace period has elapsed.
 */
void synchronize_rcu_tasks(void)
{
	/* Complain if the scheduler has not started.  */
	RCU_LOCKDEP_WARN(rcu_scheduler_active == RCU_SCHEDULER_INACTIVE,
			 "synchronize_rcu_tasks called too soon");

	/* Wait for the grace period. */
	wait_rcu_gp(call_rcu_tasks);
}
EXPORT_SYMBOL_GPL(synchronize_rcu_tasks);

/**
 * rcu_barrier_tasks - Wait for in-flight call_rcu_tasks() callbacks.
 */
void rcu_barrier_tasks(void)
{
	synchronize_rcu_tasks();
}
EXPORT_SYMBOL_GPL(rcu_barrier_tasks);

/* See if tasks are still holding out, complain if so. */
static void check_holdout_task(struct task_struct *t,
			       bool needreport, bool *firstreport)
{
	int cpu;

	if (!READ_ONCE(t->rcu_tasks_holdout) ||
	    t->rcu_tasks_nvcsw != READ_ONCE(t->nvcsw) ||
	    !READ_ONCE(t->on_rq) ||
	    (IS_ENABLED(CONFIG_NO_HZ_FULL) &&
	     !is_idle_task(t) && t->rcu_tasks_idle_cpu >= 0)) {
		WRITE_ONCE(t->rcu_tasks_holdout, false);
		list_del_init(&t->rcu_tasks_holdout_list);
		put_task_struct(t);
		return;
	}
	rcu_request_urgent_qs_task(t);
	if (!needreport)
		return;
	if (*firstreport) {
		pr_err("INFO: rcu_tasks detected stalls on tasks:\n");
		*firstreport = false;
	}
	cpu = task_cpu(t);
	pr_alert("%p: %c%c nvcsw: %lu/%lu holdout: %d idle_cpu: %d/%d\n",
		 t, ".I"[is_idle_task(t)],
		 "N."[cpu < 0 || !tick_nohz_full_cpu(cpu)],
		 t->rcu_tasks_nvcsw, t->nvcsw, t->rcu_tasks_holdout,
		 t->rcu_tasks_idle_cpu, cpu);
	sched_show_task(t);
}

/* RCU-tasks kthread that detects grace periods and invokes callbacks. */
static int __noreturn rcu_tasks_kthread(void *arg)
{
	unsigned long flags;
	struct task_struct *g, *t;
	unsigned long lastreport;
	struct rcu_head *list;
	struct rcu_head *next;
	LIST_HEAD(rcu_tasks_holdouts);
	int fract;

	housekeeping_affine(current, HK_FLAG_RCU);

	for (;;) {
		raw_spin_lock_irqsave(&rcu_tasks_cbs_lock, flags);
		list = rcu_tasks_cbs_head;
		rcu_tasks_cbs_head = NULL;
		rcu_tasks_cbs_tail = &rcu_tasks_cbs_head;
		raw_spin_unlock_irqrestore(&rcu_tasks_cbs_lock, flags);

		if (!list) {
			wait_event_interruptible(rcu_tasks_cbs_wq,
						 rcu_tasks_cbs_head);
			if (!rcu_tasks_cbs_head) {
				WARN_ON(signal_pending(current));
				schedule_timeout_interruptible(HZ/10);
			}
			continue;
		}

		synchronize_sched();

		rcu_read_lock();
		for_each_process_thread(g, t) {
			if (t != current && READ_ONCE(t->on_rq) &&
			    !is_idle_task(t)) {
				get_task_struct(t);
				t->rcu_tasks_nvcsw = READ_ONCE(t->nvcsw);
				WRITE_ONCE(t->rcu_tasks_holdout, true);
				list_add(&t->rcu_tasks_holdout_list,
					 &rcu_tasks_holdouts);
			}
		}
		rcu_read_unlock();

		synchronize_srcu(&tasks_rcu_exit_srcu);

		lastreport = jiffies;
		fract = 10;

		for (;;) {
			bool firstreport;
			bool needreport;
			int rtst;
			struct task_struct *t1;

			if (list_empty(&rcu_tasks_holdouts))
				break;

			schedule_timeout_interruptible(HZ/fract);

			if (fract > 1)
				fract--;

			rtst = READ_ONCE(rcu_task_stall_timeout);
			needreport = rtst > 0 &&
				     time_after(jiffies, lastreport + rtst);
			if (needreport)
				lastreport = jiffies;
			firstreport = true;
			WARN_ON(signal_pending(current));
			list_for_each_entry_safe(t, t1, &rcu_tasks_holdouts,
						rcu_tasks_holdout_list) {
				check_holdout_task(t, needreport, &firstreport);
				cond_resched();
			}
		}

		synchronize_sched();

		while (list) {
			next = list->next;
			local_bh_disable();
			list->func(list);
			local_bh_enable();
			list = next;
			cond_resched();
		}
		schedule_timeout_uninterruptible(HZ/10);
	}
}

static int __init rcu_spawn_tasks_kthread(void)
{
	struct task_struct *t;

	t = kthread_run(rcu_tasks_kthread, NULL, "rcu_tasks_kthread");
	BUG_ON(IS_ERR(t));
	smp_mb();
	WRITE_ONCE(rcu_tasks_kthread_ptr, t);
	return 0;
}
core_initcall(rcu_spawn_tasks_kthread);

void exit_tasks_rcu_start(void)
{
	preempt_disable();
	current->rcu_tasks_idx = __srcu_read_lock(&tasks_rcu_exit_srcu);
	preempt_enable();
}

void exit_tasks_rcu_finish(void)
{
	preempt_disable();
	__srcu_read_unlock(&tasks_rcu_exit_srcu, current->rcu_tasks_idx);
	preempt_enable();
}

#endif /* #ifdef CONFIG_TASKS_RCU */

#ifndef CONFIG_TINY_RCU

static void __init rcu_tasks_bootup_oddness(void)
{
#if defined(CONFIG_TASKS_RCU) || defined(CONFIG_TASKS_TRACE_RCU)
	if (rcu_task_stall_timeout != RCU_TASK_STALL_TIMEOUT)
		pr_info("\tTasks-RCU CPU stall warnings timeout set to %d (rcu_task_stall_timeout).\n", rcu_task_stall_timeout);
#endif
#ifdef CONFIG_TASKS_RCU
	pr_info("\tTrampoline variant of Tasks RCU enabled.\n");
#endif
#ifdef CONFIG_TASKS_TRACE_RCU
	pr_info("\tTracing variant of Tasks RCU enabled.\n");
#endif
}

#endif /* #ifndef CONFIG_TINY_RCU */

#ifdef CONFIG_TASKS_TRACE_RCU
void call_rcu_tasks_trace(struct rcu_head *rhp, rcu_callback_t func);
DEFINE_RCU_TASKS(rcu_tasks_trace, rcu_tasks_wait_gp, call_rcu_tasks_trace,
		 "RCU Tasks Trace");

void call_rcu_tasks_trace(struct rcu_head *rhp, rcu_callback_t func)
{
	call_rcu_tasks_generic(rhp, func, &rcu_tasks_trace);
}
EXPORT_SYMBOL_GPL(call_rcu_tasks_trace);

void rcu_read_unlock_trace_special(struct task_struct *t)
{
	WRITE_ONCE(t->trc_reader_need_end, false);
	if (atomic_dec_and_test(&trc_n_readers_need_end))
		wake_up(&trc_wait);
}
EXPORT_SYMBOL_GPL(rcu_read_unlock_trace_special);
#endif /* #ifdef CONFIG_TASKS_TRACE_RCU */
