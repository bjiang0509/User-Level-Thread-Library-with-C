#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "queue.h"

typedef struct node* node_t;

struct node{
	void *data_address;
	struct node *next_node;
};


struct queue {
	int queue_size;
	node_t head;
	node_t tail;

};

queue_t queue_create(void)
{
	queue_t que = (queue_t)malloc(sizeof(struct queue));
	que->queue_size = 0;
	que->head = NULL;
	que->tail = NULL;
	return que;
}

int queue_destroy(queue_t queue)
{
	/*return -1 if the queue is NULL or if the queue is not empty*/
	if(queue == NULL || queue->head != NULL || queue->tail != NULL){
		return -1;
	}
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{

	if(queue == NULL || data == NULL){
		return -1;
	}

	node_t new_node = (node_t)malloc(sizeof(struct node));
	new_node->data_address = data;
	new_node->next_node = NULL;

	if(queue->queue_size == 0){
		/*If the item to be enqueue is the first item*/
		queue->head = new_node;
		queue->tail = new_node;
	}else{
		queue->tail->next_node = new_node;
		queue->tail = new_node;
	}
	queue->queue_size++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(queue == NULL || queue_length(queue) == 0 || queue->head->data_address == NULL || queue->head == NULL){
		return -1;
	}

	/*FIFO requires that head node will be dequeue everytime*/
	node_t dequeue_node;
	if(queue_length(queue) == 1){
		dequeue_node = queue->head;
		*data = dequeue_node->data_address;
		queue->head = NULL;
		queue->tail = NULL;
		queue->queue_size = 0;
	}else{
		dequeue_node = queue->head;
		*data = dequeue_node->data_address;
		queue->head = dequeue_node->next_node;
		queue->queue_size = queue->queue_size - 1;
	}
	return 0;	
}

int queue_delete(queue_t queue, void *data)
{
	if(queue == NULL || data == NULL){
		return -1;
	}

	bool data_found = false;
	node_t previous_node;
	node_t current_node;

	previous_node = NULL; 
	current_node = queue->head;
	for(int i = 0; i < queue_length(queue); ++i){
		if(current_node->data_address == data){
			data_found = true;
			break;
		}
		previous_node = current_node;
		current_node = previous_node->next_node;
	}

	/*if data is found, current node is the node to be deleted*/
	if(data_found == false){
		return -1;
	}

	if(previous_node == NULL && current_node->next_node != NULL){
		/*the node to be deleted is the first node*/
		queue->head = current_node->next_node;
		free(current_node);
		queue->queue_size--;
	}else if(previous_node != NULL && current_node->next_node == NULL){
		/*the node to be deleted is the last node*/
		queue->tail = previous_node;
		free(current_node);
		queue->queue_size--;
	}else if(previous_node == NULL && current_node->next_node == NULL){
		/*the node to be deleted is the only node in the queue*/
		queue->head = NULL;
		queue->tail = NULL;
		free(current_node);
		queue->queue_size = 0;
	}else{
		previous_node->next_node = current_node->next_node;
		current_node->next_node = NULL;
		free(current_node);
		queue->queue_size--;
	}
	return 0;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	if(queue == NULL || func == NULL){
		return -1;
	}

	node_t node_iterator = queue->head;
	node_t node_iterator_2 = queue->head;
	for(int i = 0; i < queue_length(queue); ++i){
		func(node_iterator->data_address);
		node_iterator = node_iterator_2->next_node;
		node_iterator_2 = node_iterator;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	if(queue == NULL){
		return -1;
	}
	return queue->queue_size;
}