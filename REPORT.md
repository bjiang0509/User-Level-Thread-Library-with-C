# ECS 150 FQ 2021, Project 2: User Level Thread Library

## Authors
Name: Bijun Jiang, Student ID: 914728892
Name: Xiaoxiao(Annie) Qin, Student ID:914774707

## Phase 1: Queue

* The queue is implemented by the usage of a linked list. 
* We created a struct node, which contains two members ```void *data_address;``` and ```struct node *next_node```. node_t is a node pointer.
* There three members in ```struct queue``` are the following:
```
	int queue_size; 
	node_t head; // pointer to first node
	node_t tail; // pointer to last node
```

### Create and Destroy
* In queue_create, we use malloc to allocate space fot the queue and initialize the queue with size 0, a NULL pointer to head node and tail node.
* In queue_destroy, we call free on the queue if the queue is empty.

### Enqueue, Dequeue, and Delete
* In queue_enqueue(), we create a new_node and set its data_address to the parameter data and its next_node to node. If the queue's size is 0, both the head and tail will point to the new_node, because it is a empty queue. Else, we append the new_node to the end of the queue by setting ```queue->tail->next_node = new_node``` and the new_node becomes the new tail to the queue.
* In queue_dequeue(), we check the length of the queue first. If there's only one element, dequeue will result in a empty queue. In this case, we just set both head and tail pointer to NULL and the queue size to 0. If there's more than one element, we set ```*data = dequeue_node->data_address```. The dequeue_node's next_node becomes the new head due to FIFO requirement. Queue size decrease by 1.
* In queue_delete(), we create two node_t variables to keep track of the previous and current node. The current node will be first set to the head. Then we enter a for loop that iterate through the queue to check if the current_node's data matches the data to be deleted. If there's a match, we set the bool variable data_found to true and break. Else, previous_node becomes the current_node and the current_node become its next node. If the data was found, there are four possible cases:
1. The node to be deleted is the first node. So the head becomes the current node's next node and we free the current node.
2. The node to be deleted is the last node. So the tail becomes the previous node and we free the current node.
3. The node to be deleted is the only node in the queue. so we set head and tail to NULL and free the node.
4. The node is in between the first and last, so the we set the previous node's next node to the current node's next node. The current node's next_node becomes NULL and we free the node.

### Iterate and Length
* Using the same iteration strategy in queue_delete, we iterate through each node and call func with each node's data_address memeber.
* The length just returns queue's size.

## Phase 2: Uthread
Our implementation of the Uthread API requires two global variable.
```
	queue_t thread_que;
	struct uthread_tcb *current_thread; // the current thread
```
* We created a ```struct thread_state``` to keep track of the thread's four states: ready, running, blocked, exit.
* The uthread_tcb struct includes four members as follows:
```
	struct thread_state state; // state of the thread
	void *stack; // stack of the thread
	uthread_ctx_t *ctx_thread; // contexts of the thread
	uthread_func_t = thread_function; // function to be run by the thread
```

### Start, Create, and Exit
* In uthread_start, we first create a queue for the threads (thread_que). Next, we create the idle thread and set its running state to true and other states to false. Then, we set the current_thread to the idle_thread, allocate its stack by calling ```uthread_ctx_alloc_stack()```, and call ```uthread_ctx_init``` to start the thread. Lastly, the idle thread will continue to yield as long as the queue's length is greater than 0.
* In uthread_create, we use malloc to allocate space for the new thread. Next, we set the new thread's ready state to true and allocate a stack for it. Then, we initialize and enqueue this new thread onto the thread queue (thread_que).
* In uthread_exit, we call free on the current_thread's context and destroy its stack by calling ```uthread_ctx_destroy_stack```. Then we set its state to exit and yield to the next thread.

### Yield
* If the running state of the current thread is true, we set it to false and set ready to true. Then, we put the thread back onto thread_queue with queue_enqueue. 
* We grab a new thread from the thread queue with queue_dequeue and save the state of the current thread with ```struct uthread_tcb *save_state = uthread_current()```. 
* If the new thread's ready state is true, we set the new thread's running state to true and set it to be the current thread. Then, we call ```uthread_ctx_switch``` to switch. 
* Else, we set the current thread to the new thread, perform context switch
from the save_state thread's context to the new thread's, and yield again.

### Block and Unblock
* In uthread_block, we set the current_thread's blocked state to true and yield.
* In uthread_unblock, we set the current_thread's ready state to true and put it back onto the thread queue.

## Semaphore
Our semaphore struct have the following members:
```
	size_t internal_count;
	struct queue *threads;
```
### Create and Destroy
* In sem_create, we use malloc to allocate space, initialize the internal count to count and create a threads queue with queue_create().
* In sem_destroy, we call queue_destroy on the threads queue and free the space taken up by sem.

### Up and Down
* Lecture 08.sync is our main source for writing sem_up and sem_down.
* In sem_up, we increased the internal count by 1. If there are threads in the threads queue, we grab the first thread with dequeue and unblock it (allowing it to run).
* In sem_down, we put the current thread on the thread queue through enqueue. While the sem's internal count is 0, we block the thread by calling uthread_block(). When the internal count is no longer 0, the internal count will decrease by 1 and returns 0.

## Preempt
* Lecture 03.syscall provided a helpful example on setting up a alarm signal handler
* The global variables listed below are essential for our implementation
```
	struct sigaction sa;
	struct sigaction prev_sa;
	structt itimerval new, old;
```

### Start and Stop
* In preempt_start
1. we set up a signal handler with the alarm handler we wrote (calls uthread_yield). 
2. We initialize the sigset member of sa (sa_mask) with sigemptyset and sigaddset and set sa flag to 0;
3. By calling ```sigaction(SIGVTALRM, &sa, &prev_sa)```, the alarm handler in sa.sa_handler will carry out upon receiving SIGVTLRM signal and the previous configuration will be save to prev_sa.
4. The itimerval variable new.it_interval.tv_usec and new.it_value.tv_usec are set to HZ_MICROSEC (10000 microseconds)
5. We call ```setitimer(ITIMER_VIRTUAL, &new, &old)``` to set a timer counting down in the time specify by new and store the previous timer value in old.
* In preempt_stop, we disable preempt and set the current sa to prev_sa and new to old in order to restore previous configuration.

### Enable and Disable
* In preempt_enable, we initialize sa.sa_mask with SIGVTALRM and make sure that we will receive(unblocking) the signals by calling ```sigprocmask(SIG_UNBLOCK, &sa.sa_mask, &prev_sa.sa_mask);```
* In preempt_disable, we have the same codes, but SIGBLOCK instead of SIG_UNBLOCK

## Testing
We wrote a queue_tester to test all the features of phase 1. We made sure that we pass all the provided test cases for phase 2 and 3. We even wrote a sem_corner_case.c to test the corner case for semaphore. For the last phase, we wrote a test_preempt.c to test the ability of preempt. The preempt tester never calls uthread_yield with every thread in a infinite loop. It proves that out preempt.c works, because the program still manages to reach all threads without calling yield.

## Sources
1. https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
2. http://crasseux.com/books/ctutorial/Linked-lists.html
3. https://www.gnu.org/software/libc/manual/html_node/Sigaction-Function-Example.html
4. https://www.gnu.org/software/libc/manual/html_node/Signal-Sets.html
5. https://www.gnu.org/software/libc/manual/html_node/Setting-an-Alarm.html
6. Lecture on syscalls and the PDF 03.syscall