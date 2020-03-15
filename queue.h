#include <stdio.h>
#ifndef QUEUE_H
#define QUEUE_H
typedef struct node_struct{
	void* value;
}node;

typedef struct queue_struct{
	node* list;
	int front;
	int rear;
	int size;
}queue;

int q_init(queue *q, int size);
int q_empty(queue *q);
int q_full(queue *q);
int q_push(queue* q, void* value);
void *q_pop(queue *q);
void q_clean(queue *q);

#endif
