#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"queue.h"
struct Global
{
	pthread_mutex_t queue_lock;
	pthread_mutex_t service_lock;
	pthread_mutex_t result_lock;
	queue* sharedarray;
	int file_count;
	FILE** fileArray;
	int helpFiles;
	int* noLiveRequester;
};
struct reqThread
{
	int count;
	FILE* filename;
	struct Global* global;
	FILE* serviced;
	int id;
}; 
struct resThread
{
	struct Global* global;
	FILE* results;
};
