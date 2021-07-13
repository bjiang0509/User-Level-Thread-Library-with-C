#include <stddef.h>
#include <stdlib.h>

#include "queue.h"
#include "sem.h"
#include "uthread.h"
#include "private.h"

struct semaphore {
	size_t internal_count;
	struct queue *threads;
};

sem_t sem_create(size_t count)
{
	preempt_start();
	preempt_disable();
	struct semaphore *sem = (sem_t)malloc(sizeof(struct semaphore));
	sem->threads = queue_create();
	preempt_enable();
	sem->internal_count = count;
	return sem;
}

int sem_destroy(sem_t sem)
{
	preempt_disable();
	int destroy_stat = queue_destroy(sem->threads);
	preempt_enable();
	if(destroy_stat == 0){
		free(sem);
		return 0;
	}

	return -1;
}

int sem_down(sem_t sem)
{
	/*source: lecture 08.sync*/
	if(sem == NULL){
		return -1;
	}
	if(sem->internal_count == 0){
		struct uthread_tcb *waiting_thread = uthread_current();
		preempt_disable();
		queue_enqueue(sem->threads, waiting_thread);
		preempt_enable();
	}
	/*thread will remain blocked if the internal count is still 0*/
	while(sem->internal_count == 0){
		uthread_block();
	}

	sem->internal_count -= 1;
	return 0;
}

int sem_up(sem_t sem)
{

	if(sem == NULL){
		return -1;
	}
	/*internal count increase by one and the first thread gets unblocked*/
	sem->internal_count += 1;
	if(queue_length(sem->threads) > 0){
		struct uthread_tcb *first_thread;
		preempt_disable();
		queue_dequeue(sem->threads, (void**)&first_thread);
		preempt_enable();
		uthread_unblock(first_thread);
	}
	return 0;
}


