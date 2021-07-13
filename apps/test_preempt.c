#include <stdio.h>
#include <stdlib.h>

#include "uthread.h"

/*this tester does not call uthread_yield();
	however, thread2 and thread3 are still able to print due
	to preempt*/
void thread3(void *arg)
{
	printf("\n");
	printf("Reached thread3: Exit\n");
	exit(0);
}

void thread2(void *arg)
{
	printf("\n");
	while(1){
		printf("xxxxxxxxxxxxxxxxx");
	}
}

void thread1(void *arg)
{
	printf("\n");
	uthread_create(thread2, NULL);
	uthread_create(thread3, NULL);

	while(1){
		printf("thread1 ");
	}
}

int main(void)
{
	uthread_start(thread1, NULL);
	return 0;
}
