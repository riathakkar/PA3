
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/time.h>
#include"util.h"
#include"multi-lookup.h"
#include"queue.h"

#define MaxInputFiles 10
#define MaxResolverThreads 10
#define MaxRequesterThreads 5
#define MaxNameLength 1025
#define MaxIpLength INET6_ADDRSTRLEN
void* requester(void* param){
	char hostnames[MaxNameLength];
	struct req_thread_info* thread = param;
	char *temp;
	FILE* fileptr;
	int done = 0;
	while(thread->fileOn < thread->global->file_count)
	{
		if(thread->global->fileArray[thread->fileOn] != NULL)
		{
			thread->filename = thread->global->fileArray[thread->fileOn];
			/*if(!thread->filename){
				fprintf(stderr, "Invalid file %s", thread->filename);
				thread->fileOn+=1;
			}*/
			while(fscanf(thread->filename, " %1024s ", hostnames) > 0)
					{
						while(done == 0)
						{
							pthread_mutex_lock(&thread->global->queue_lock);
							if(q_full(thread->global->shared_array) == 0)
							{
								fprintf(stdout,"%s", hostnames);
								temp = malloc(MaxNameLength);
								strncpy(temp, hostnames, MaxNameLength);
								q_push(thread->global->shared_array, temp);
								done = 1;
								free(temp);
							}
							else
							{
								
								done = 0;
								free(temp);
							}
							pthread_mutex_unlock(&thread->global->queue_lock);
						}
						done = 0;
				}
				
			}
			thread->count += 1;
			thread->fileOn += 1;
	}
	
	pthread_mutex_lock(&thread->global->service_lock);

	if(thread->serviced != NULL)
	{
		if(thread->count != 0)
		{
			fprintf(thread->serviced, "thread %d worked on %d\n", thread->id, thread->count);
		}
		else
		{
			fprintf(thread->serviced, "thread %d worked on 0\n", thread->id);
		}
	}
	
	pthread_mutex_unlock(&thread->global->service_lock);
	
	return NULL;
}
void* resolver(void* p)
{
	struct res_thread_info* thread = p;
	char hostnames[MaxNameLength];
	char ipaddress[INET6_ADDRSTRLEN];
	int len = sizeof(ipaddress);
	char* temp;
	while(q_empty(thread->global->shared_array) == 0 || *thread->global->running == 1)
	{
		pthread_mutex_lock(&thread->global->queue_lock);
		temp = q_pop(thread->global->shared_array);
		if(temp == NULL)
		{
			pthread_mutex_unlock(&thread->global->queue_lock);
			usleep(rand() % 100);
		} else {
			pthread_mutex_unlock(&thread->global->queue_lock);
			if(dnslookup(temp, ipaddress, len) == UTIL_FAILURE){
				strncpy(ipaddress, "", MaxNameLength);
				fprintf(stderr, "Error: DNS lookup failure on hostname: %s\n", temp);	
			}
			pthread_mutex_lock(&thread->global->result_lock);
			fprintf(thread->results, "%s,%s\n", temp,ipaddress);
			// Unlock  output file after access
			pthread_mutex_unlock(&thread->global->result_lock);
		}
		
	}
	return NULL;
}
int main(int argc, char* argv[]){
	struct timeval start, end;
	struct Global global;
	gettimeofday(&start,NULL);

	if(argc > (MaxInputFiles + 5))
	{
		return -1;
	}
	FILE* service_file;
	FILE* result_file;
	queue shared_array;
	int number_files  = argc-5;
	int request_count = atoi(argv[1]);
	int resolve_count = atoi(argv[2]);
	service_file      = fopen(argv[3], "w");
	result_file       = fopen(argv[4], "w");
	int running       = 1;
	FILE* in_file[number_files];
	struct req_thread_info request_t[request_count];
	struct res_thread_info resolve_t[resolve_count];
	pthread_t req_threads[request_count];
	pthread_t res_threads[resolve_count];

	//error checks
	if(argc < 6) {
    	fprintf(stderr, "Too few arguments.\n");
    	return 0;
  	}
  	if(atoi(argv[1]) > MaxRequesterThreads || atoi(argv[2]) > MaxResolverThreads) {
    	fprintf(stderr, "Too many threads. Can't be over 10 for either type.\n");
    	return 0;
  	}
  	if(argc-5 > MaxInputFiles) {
    	fprintf(stderr, "Cant read more than 10 name files.\n");
    	return 0;
  	}
	if(q_init(&shared_array, MaxNameLength) == -1) fprintf(stderr, "Queue init failed");

	for(int i=0; i<number_files; i++){
		in_file[i] = fopen(argv[5+i], "r");
	}

	//pthread initialization
	pthread_mutex_init(&global.queue_lock, NULL);
	pthread_mutex_init(&global.service_lock, NULL);
	pthread_mutex_init(&global.result_lock, NULL);
	pthread_cond_init(&global.cond_request, NULL);
	pthread_cond_init(&global.cond_resolve, NULL);
	//global base initialization
	global.shared_array = &shared_array;
	global.file_count   = number_files;
	global.running      = &running;
	global.fileArray    = in_file;
	//creating requster struct
	int error = 0;
	for(int i = 0; i < request_count; i++){
		if(i > number_files){
			request_t[i].fileOn = 0;
		}else{
			request_t[i].fileOn = i;
		}
		request_t[i].global   = &global;
		request_t[i].id       = i;
		request_t[i].count    = 0;
		request_t[i].serviced = service_file;
		request_t[i].filename = in_file[i];
		error = pthread_create(&req_threads[i], NULL, requester, &request_t[i]);
		if(error != 0){
			fprintf(stderr, "Failed to create pthread\n");
		}
	}
	error = 0;
	for(int i = 0; i < resolve_count; i++){
		resolve_t[i].global  = &global;
		resolve_t[i].results = result_file;
		error = pthread_create(&(res_threads[i]), NULL, resolver, &(resolve_t[i]));
		if(error != 0){
			fprintf(stderr, "Failed to create pthread");
		}
	}
	for(int i = 0; i < request_count; i++)
	{
		pthread_join(req_threads[i], NULL);
	}
	running = 0;
	for(int i = 0; i < resolve_count; i++)
	{
		pthread_join(res_threads[i], NULL);
	}

	pthread_mutex_destroy(&global.queue_lock);
	pthread_mutex_destroy(&global.service_lock);
	pthread_mutex_destroy(&global.result_lock);
	pthread_cond_destroy(&global.cond_request);
	pthread_cond_destroy(&global.cond_resolve);
	q_clean(&shared_array);
	fclose(result_file);
	fclose(service_file);
	for(int i =0; i < number_files; i++){
	if(in_file[i]){
	fclose(in_file[i]);
	}
	}
	gettimeofday(&end, NULL);
	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
	printf("Multi-Lookup took %ld seconds and %ld micros to complete\n", seconds, micros);

	return 0;
}
