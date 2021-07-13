//Thread A calls down() on a semaphore with a count of 0, and gets blocked.
//Thread B calls up() on the same semaphore, and gets thread A to be awaken
//Before thread A can run again, thread C calls down() on the semaphore and snatch the newly available resource.
//When A finally gets to run, it should correctly carry out its job

#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"
#include "sem.h"

sem_t semA;

void threadF(void *arg)
{
	sem_up(semA);
	printf("threadF\n");
}

void threadE(void *arg){
	sem_down(semA);
	printf("threadE\n");
}

void threadD(void *arg){
	sem_up(semA);
	printf("threadD\n");
}

void threadC(void *arg)
{
	sem_down(semA);
	printf("threadC\n");
}

void threadB(void *arg)
{
	sem_up(semA);
	printf("threadB\n");
}

void threadA(void *arg)
{
	uthread_create(threadB, NULL);
	uthread_create(threadC, NULL);
	uthread_create(threadD, NULL);
	uthread_create(threadE, NULL);
	uthread_create(threadF, NULL);

	sem_down(semA);
	printf("threadA\n");
	
}

int main(void)
{
	semA = sem_create(0);
	uthread_start(threadA, NULL);
	return 0;
}