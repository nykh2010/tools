/*
 * main.c
 *
 *  Created on: Mar 30, 2018
 *      Author: root
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>

#define MAX_QUEUE_BLOCK		10

typedef struct {
	int size;
	void *buff;
} element_t;

typedef struct {
	sem_t sem;
	int head;
	int tail;
	element_t element[MAX_QUEUE_BLOCK];
} queue_t;

int move_next_pos(int pos) {
	return pos+1 < MAX_QUEUE_BLOCK? pos+1 : 0;
}

int wait_queue_not_full(queue_t *queue) {
	int sval;
	sem_getvalue(&queue->sem, &sval);
	if (sval == MAX_QUEUE_BLOCK) {
		/* full */
		return false;
	}
	return true;
}

int wait_queue_not_empty(queue_t *queue) {
	struct timespec timeout;
	timeout.tv_sec = 1;
	timeout.tv_nsec = 0;
	int ret = sem_timedwait(&queue->sem, &timeout);
	if (ret < 0) {
		return false;
	}
	return true;
}

int read_element_from_queue(queue_t *queue, element_t *element) {
	if (wait_queue_not_empty(queue) == false) {
		return false;
	}
	element->buff = queue->element[queue->head].buff;
	element->size = queue->element[queue->head].size;
	queue->head = move_next_pos(queue->head);
	return true;
}

int write_element_into_queue(queue_t *queue, element_t *element) {
	if (wait_queue_not_full(queue) != true) {
		return false;
	}
	queue->element[queue->tail].buff = element->buff;
	queue->element[queue->tail].size = element->size;
	queue->tail = move_next_pos(queue->tail);
	printf("write %d\n",*((int *)element->buff));
	sem_post(&queue->sem);
	return true;
}

int init_queue(queue_t *queue, int max_sem_value) {
	int ret;
	ret = sem_init(&queue->sem, 0, 0);
	queue->head = 0;
	queue->tail = 0;
	return true;
}

int main(int argc, char **argv) {
	int i;
	element_t element, element_out;
	queue_t queue;
	init_queue(&queue, MAX_QUEUE_BLOCK);
	for (i=0;i<20;i++) {
		element.buff = (int *)malloc(sizeof(int));
		*((int *)element.buff) = i;
		element.size = sizeof(int);
		if (write_element_into_queue(&queue,&element) == false) {
			break;
		}
	}
	while(read_element_from_queue(&queue, &element_out)) {
		printf("element_out: %d\n",*((int *)element_out.buff));
		free(element_out.buff);
	}
	exit(0);
}
