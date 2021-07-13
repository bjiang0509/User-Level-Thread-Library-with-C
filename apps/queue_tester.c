#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

/*Test Queue Iteration*/
int add_one(void *data){
	*(int*)data = *(int*)data + 1;
	return 0;
}
void test_queue_iteration_and_length(void){
	int data_1 = 1, data_2 = 2, data_3 = 3;
	queue_t q;

	fprintf(stderr, "*** TEST queue_iteration_and_length ***\n");
	q = queue_create();
	queue_enqueue(q, &data_1);
	queue_enqueue(q, &data_2);
	queue_enqueue(q, &data_3);
	queue_func_t add_ftn = (queue_func_t)&add_one;
	queue_iterate(q, add_ftn);
	TEST_ASSERT(queue_length(q) == 3);
	TEST_ASSERT(data_1 == 2);
	TEST_ASSERT(data_2 == 3);
	TEST_ASSERT(data_3 == 4);
}

/*Test Queue Delete*/
void test_queue_delete(void){
	queue_t q;
	int data_1 = 1, data_2 = 2, data_3 = 3, *ptr;

	fprintf(stderr, "*** TEST queue_delete_middle ***\n");
	q = queue_create();
	queue_enqueue(q, &data_1);
	queue_enqueue(q, &data_2);
	queue_enqueue(q, &data_3);
	queue_delete(q, &data_2);
	TEST_ASSERT(queue_length(q) == 2);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data_1);
}

void test_queue_delete_first(void){
	queue_t q;
	int data_1 = 1, data_2 = 2, data_3 = 3, *ptr;

	fprintf(stderr, "*** TEST queue_delete_first ***\n");
	q = queue_create();
	queue_enqueue(q, &data_1);
	queue_enqueue(q, &data_2);
	queue_enqueue(q, &data_3);
	queue_delete(q, &data_1);
	TEST_ASSERT(queue_length(q) == 2);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data_2);
}

void test_queue_delete_last(void){
	queue_t q;
	int data_1 = 1, data_2 = 2, data_3 = 3, *ptr;

	fprintf(stderr, "*** TEST queue_delete_last ***\n");
	q = queue_create();
	queue_enqueue(q, &data_1);
	queue_enqueue(q, &data_2);
	queue_enqueue(q, &data_3);
	queue_delete(q, &data_3);
	TEST_ASSERT(queue_length(q) == 2);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data_1);
}

void test_queue_destroy(void){
	queue_t q;
	fprintf(stderr, "*** TEST queue_destroy ***\n");
	q = queue_create();
	int stat = queue_destroy(q);
	TEST_ASSERT(stat == 0);
}

void test_queue_errors(void){
	queue_t q;
	int data_1 = 1, data_3 = 4, *ptr;
	int *data_2 = NULL;
	fprintf(stderr, "*** TEST queue_errors ***\n");

	q = NULL;
	/*error that accompanies a NULL queue*/
	int stat_1 = queue_destroy(q);
	TEST_ASSERT(stat_1 == -1);
	int stat_2 = queue_enqueue(q, &data_1);
	TEST_ASSERT(stat_2 == -1);
	int stat_3 = queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(stat_3 == -1);
	int stat_4 = queue_delete(q, &data_1);
	TEST_ASSERT(stat_4 == -1);
	int stat_5 = queue_length(q);
	TEST_ASSERT(stat_5 == -1);
	queue_func_t add_ftn = (queue_func_t)&add_one;
	int stat_6 = queue_iterate(q, add_ftn);
	TEST_ASSERT(stat_6 == -1);

	q = queue_create();
	/*attempt to destroy unempty queue*/
	queue_enqueue(q, &data_1);
	int stat_7 = queue_destroy(q);
	TEST_ASSERT(stat_7 == -1);
	int stat_8 = queue_enqueue(q, data_2);
	TEST_ASSERT(stat_8 == -1);
	queue_dequeue(q, (void**)&ptr);
	/*empty queue at this point*/
	int stat_9 = queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(stat_9 == -1);
	queue_enqueue(q, &data_1);
	/*delete data not in queue*/
	int stat_10 = queue_delete(q, &data_3);
	TEST_ASSERT(stat_10 == -1);
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_queue_iteration_and_length();
	test_queue_delete();
	test_queue_delete_first();
	test_queue_delete_last();
	test_queue_destroy();
	test_queue_errors();

	return 0;
}
