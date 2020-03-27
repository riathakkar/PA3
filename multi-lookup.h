#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MaxInputFiles 10
#define MaxResolverThreads 10
#define MaxRequesterThreads 5
#define MaxNameLength 1025
#define MaxIpLength INET6_ADDRSTRLEN
#include"queue.h"
struct Globals
{
	pthread_mutex_t queue_lock;
	pthread_mutex_t service_lock;
	pthread_mutex_t result_lock;
  pthread_mutex_t input_lock[MaxInputFiles];
	pthread_cond_t cond_request;
	pthread_cond_t cond_resolve;
	queue shared_array;
	int file_count;
        FILE* fileArray[MaxInputFiles];
        char file_status[MaxInputFiles];
  pthread_t req_threads[MaxRequesterThreads];
  pthread_t res_threads[MaxResolverThreads];
  FILE *results;
  FILE *serviced;
  int fileOn;
};

void *requester(void *);
