#include "queue.h"
#include <stdlib.h>

int q_init(queue*q, int size){
	if(size > 0){
		q->size = size;
	}else{
		printf("queue size error");
		return -1;
	}
	q->list = malloc(sizeof(node) * size);
	if(!(q->list)){
		printf("allcation error");
		return -1;
	}
	for(int i=0; i<size;++i){
		q->list[i].value = NULL;
	}
	q->front = 0;
	q->rear  = 0;
	return q->size;
}

int q_empty(queue*q){
	if((q->front == q->rear) && (q->list[q->front].value == NULL)){
		return 1;
	}else{
		return 0;
	}
}



int q_push(queue *q, void* input_value){
	if(q_full(q) == 1){
		printf("queue is full");
		return -1;
	}
	q->list[q->rear].value = input_value;
	q->rear = q->rear+1;
	return 0;
}
void *q_pop(queue *q){
	void * pop_value;
	if(q_empty(q)==1){
		return NULL;
	}
	pop_value = q->list[q->front].value; //pop the first of the list
	q->list[q->front].value = NULL;
	q->front = q->front +1;
	return pop_value;
}

int q_full(queue *q){
	if((q->front == q->rear) && (q->list[q->front].value != NULL)){
		return 1;
	}else{
		return 0;
	}
}

void q_clean(queue *q){
	while(q_empty(q)!=1){
		q_pop(q);
	}
	free(q->list);
}
