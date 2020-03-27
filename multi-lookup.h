#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"queue.h"
struct Global
{
	pthread_mutex_t queue_lock;
	pthread_mutex_t service_lock;
	pthread_mutex_t result_lock;
	pthread_cond_t cond_request;
	pthread_cond_t cond_resolve;
	queue* shared_array;
	int file_count;
	FILE** fileArray;
	int* running;
};
struct req_thread_info
{
	int count;
	FILE* filename;
	struct Global* global;
	FILE* serviced;
	int id;
	int fileOn;
};
struct res_thread_info
{
	struct Global* global;
	FILE* results;
};
