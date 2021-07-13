#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "private.h"
#include "uthread.h"

/*
 * Frequency of preemption
 * 100Hz is 100 times per second
 */
/*sec = seconds, usec = microseconds, 100hz = 10000 microseconds*/
#define HZ 100
#define HZ_MICROSEC 10000

struct sigaction sa;
struct sigaction prev_sa;
struct itimerval new, old;

void alarm_handler(int signum){
	//handler function to thread yield
	uthread_yield();
}

void preempt_disable(void)
{
	/*souce: https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html*/
	/*Source: lecture 03.syscalls slide 43*/
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &prev_sa.sa_mask);
}

void preempt_enable(void)
{
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, &prev_sa.sa_mask);
}

void preempt_start(void)
{
	/*Install a signal handler that receives alarm signals (of type SIGVTALRM)*/
	/*source: lecture 03.syscalls slide 41/43*/
	sa.sa_handler = &alarm_handler;
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGVTALRM);
	sa.sa_flags = 0;
	sigaction(SIGVTALRM, &sa, &prev_sa);

	/*configure timer*/
	/*source: https://www.gnu.org/software/libc/manual/html_node/Setting-an-Alarm.html*/
	new.it_interval.tv_usec = HZ_MICROSEC;
	new.it_interval.tv_sec = 0;
	new.it_value.tv_usec = HZ_MICROSEC;
	new.it_value.tv_sec = 0;
	setitimer(ITIMER_VIRTUAL, &new, &old);
}

void preempt_stop(void)
{
	preempt_disable();
	/*set current sa to previous sa*/
	sa = prev_sa;
	new = old;
}

