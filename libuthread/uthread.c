#include <assert.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

#include "private.h"
#include "uthread.h"
#include "queue.h"

/*Global Block*/
queue_t thread_que;
queue_t exit_que;
struct uthread_tcb *current_thread;
/*End Global*/

struct thread_state{
	/*states for the thread*/
	bool ready;
	bool running;
	bool blocked;
	bool exit;
};

struct uthread_tcb{
	struct thread_state state;
	void *stack;
	uthread_ctx_t *ctx_thread;
};

struct uthread_tcb *uthread_current(void)
{
	return current_thread;
}

void uthread_yield(void)
{ 
	/*change state of the current thread and place it back on the queue*/
	/*exited thread will not be put back into the queue*/
	preempt_enable();
	if(current_thread->state.running == true && current_thread->state.exit == false){
		current_thread->state.ready = true;
		current_thread->state.running = false;
		current_thread->state.blocked = false;
		preempt_disable();
		queue_enqueue(thread_que, current_thread);
		preempt_enable();
	}

	/*get new thread from the queue*/
	struct uthread_tcb *new_thread;
	preempt_disable();
	int dequeue_stat = queue_dequeue(thread_que, (void**) &new_thread);
	preempt_enable();

	if(dequeue_stat == -1){
		/*problem occurred in dequeue*/
		return;
	}

	/*save current thread for context switch*/
	struct uthread_tcb *save_state = uthread_current();

	/*If new_tread is ready to run, we set it to running and call ctx_switch,
			else,  we yield to next thread*/
	if(new_thread->state.ready == true){
		new_thread->state.ready = false;
		new_thread->state.running = true;
		new_thread->state.blocked = false;
		new_thread->state.exit = false;
		current_thread = new_thread;
		uthread_ctx_switch(save_state->ctx_thread, new_thread->ctx_thread);
	}else {
		/*if the new thread is blocked, complete context switch and yield again*/
		current_thread = new_thread;
		uthread_ctx_switch(save_state->ctx_thread, new_thread->ctx_thread);
		uthread_yield();
	}
}

void uthread_exit(void)
{
	struct uthread_tcb *exit_thread = uthread_current();

	exit_thread->state.ready = false;
	exit_thread->state.running = false;
	exit_thread->state.blocked = false;
	exit_thread->state.exit = true;

	queue_enqueue(exit_que, exit_thread);

	uthread_yield();
}

int uthread_create(uthread_func_t func, void *arg)
{
	preempt_enable();
	bool prob_encountered = false;
	struct uthread_tcb *new_thread = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb));
	if(new_thread == NULL){
		prob_encountered = true;
	}

	if(prob_encountered == false){
		new_thread->ctx_thread = (uthread_ctx_t *)malloc(sizeof(uthread_ctx_t));
		if(new_thread->ctx_thread != NULL){
			/*Initialize the states of the new thread*/
			new_thread->state.ready = true;
			new_thread->state.running = false;
			new_thread->state.blocked = false;
			new_thread->state.exit = false;
			/*initialize the new thread and allocate the stack*/
			new_thread->stack = uthread_ctx_alloc_stack();
			uthread_ctx_init(new_thread->ctx_thread, new_thread->stack, func, arg);
			/*place the new thread on the queue*/
			preempt_disable();
			queue_enqueue(thread_que, new_thread);
			preempt_enable();
			return 0;
		}
	}
	return -1;
}

int uthread_start(uthread_func_t func, void *arg)
{
	/*create the queue for the threads*/
	thread_que = queue_create();
	exit_que = queue_create();
	preempt_start();
	preempt_enable();

	bool prob_encountered = false;
	struct uthread_tcb *idle_thread = (struct uthread_tcb*)malloc(sizeof(struct uthread_tcb));
	if(idle_thread == NULL){
		prob_encountered = true;
	}

	if(prob_encountered == false){
		idle_thread->ctx_thread = (uthread_ctx_t *)malloc(sizeof(uthread_ctx_t));
		if(idle_thread->ctx_thread != NULL){
			/*set the state of the idle thread to runnning and increase the count*/
			idle_thread->state.ready = false;
			idle_thread->state.running = true;
			idle_thread->state.blocked = false;
			idle_thread->state.exit = false;
			current_thread = idle_thread;

			current_thread->stack = uthread_ctx_alloc_stack();
			uthread_ctx_init(current_thread->ctx_thread, current_thread->stack, func, arg);

			int create_stat = uthread_create(func,arg);
			if(create_stat == -1){
				return -1;
			}

			/*idle thread will keep running as long as there's at least one thread in the queue*/
			while(queue_length(thread_que) > 0){
				while(queue_length(exit_que) > 0){
					/*If there are queue in the exit queue, we free their context and destroy their stack*/
					struct uthread_tcb *exit_thread;
					preempt_disable();
					queue_dequeue(exit_que, (void**)&exit_thread);
					preempt_enable();
					free(exit_thread->ctx_thread);
					uthread_ctx_destroy_stack(exit_thread->stack);
				}
				uthread_yield();
			}
			return 0;
		}
	}

	return -1;
}

void uthread_block(void)
{
	/*set the state to blocked and yield*/
	preempt_enable();
	current_thread->state.ready = false;
	current_thread->state.running = false;
	current_thread->state.blocked = true;
	current_thread->state.exit = false;
	uthread_yield();
	return;
}

void uthread_unblock(struct uthread_tcb *uthread)
{
	/*Source:https://piazza.com/class/kfhaqafstkv3l3?cid=433*/
	/*set the state to ready and place it back on the queue*/
	preempt_enable();
	if(uthread->state.blocked == false){
		return;
	}
	uthread->state.blocked = false;
	uthread->state.running = false;
	uthread->state.ready = true;
	uthread->state.exit = false;

	preempt_disable();
	queue_enqueue(thread_que, uthread);
	preempt_enable();
	/*if thread is not blocked, nothing needs to be done*/
	return;
}