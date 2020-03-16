#include<util.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include <sys/time.h>

#include"multi-lookup.h"
#include"queue.h"

#define MaxInputFiles 10
#define MaxResolverThreads 10
#define MaxRequesterThreads 5
#define MaxNameLength 1025
#define MaxIpLength INET6_ADDRSTRLEN

// int main(int argc, char *argv[]){
// 	int num_names = argc  - 5; //5 is the base argument count
// 	int requester_count = atoi(argv[1]);
// 	int resolver_count = atoi(arv[2]);
//
// 	int thread_running = 1;
//
// 	//argv[3] - results out file
// 	//argv[4] - service out file
// 	FILE* resultfile = NULL;
// 	FILE* servicefile = NULL;
// 	FILE* in_files[num_names];
// 	//create request/resolve threads
// 	pthread_t request_threads[requester_count];
// 	pthread_t resolve_threads[resolver_count];
//
// 	//create mutexes
// 	pthread_mutex_t shared_space_lock;
// 	pthread_mutex_t file_lock;
// 	pthread_mutex_t output_lock;
//
// 	queue shared_space;
//
// 	if(q_init(&shared_space, 100) == -1){
// 		fprintf(stderr, "creating shared queue error\n");
// 		return -1;
// 	}
//
// 	pthread_mutex_init(&shared_space_lock, NULL);
// 	pthread_mutex_init(&file_lock, NULL);
// 	pthread_mutex_init(&output_lock, NULL);
//
// 	struct req_thread_info req_t_info;
// 	struct res_thread_info res_t_info;
//
// 	resultfile = fopen(argv[3], "w");
// 	servicefile = fopen(argv[4], "w");
// 	//add error handling
//
// 	for(int i = 0; i < num_names; i++){
// 		in_files[i] = fopen(argv[i+5], "r");
// 		if(!in_files[i]) fprintf(stderr,"Error opening file");
// 	}
//
// 	for(int i = 0; i < num_names; i++){
// 		FILE* curFile=inputFiles[i];
// 		req_t_info[i].queue_lock = &shared_space_lock;
// 		req_t_info[i].file_name = curFile;
// 		req_t_info[i].queue = &shared_space;
// 		req_t_info[i].file_lock = &file_lock;
// 		req_t_info[i].service_name = servicefile;
// 		//add error handling
// 		req_t_info[i].num_on = i;
// 		req_t = pthread_create(&request_threads[i], NULL, requester, &req_t_info[i]);
// 	}
// 	for(int i = 0; i < resolver_count; i++){
// 		res_t_info[i].queue_lock = &shared_space_lock;
// 		res_t_info[i].file_name = resultfile;
// 		res_t_info[i].file_lock = &output_lock;
// 		res_t_info[i].queue = &shared_space;
// 		res_t_info[i].running = &thread_running;
// 		pthread_create(&resolve_threads[i], NULL, resolver, &res_t_info[i]);
// 		//add error handling
// 	}
// 	for(int i = 0; i < infiles; i++){
// 		pthread_join(request_threads[i], NULL)
// 	}
//
// 	thread_running=0;
// 	//Now we wait for the output threads to finish running
// 	for(int i=0;i<resolver_count;i++){
//         	pthread_join(resolve_threads[i],NULL);
//     	}
//
// 	fclose(resultfile);
// 	fclose(servicefile);
// 	q_clean(&shared_space);
// 	fcloseall();
//
// 	pthread_mutex_destroy(&shared_space_lock);
// 	pthread_mutex_destroy(&file_lock);
// 	pthread_mutex_destroy(&output_lock);
//
//
// 	return 0;
// }
//
// void* requester(void *p){
// 	char hostnames[MAX_NAME_LENGTH];
// 	struct* req_thread_info thread = p;
//
// 	int done = 0;
// 	while(thread->num_on <
//
// }
void* requester(void* param){
	char hostnames[MaxNameLength];
	struct* req_thread_info thread = param;
	int *temp;
	int done = 0;
	while(thread->fileOn < thread->Gspot->numFiles)
	{
		if(thread->Gspot->fileArray[thread->fileOn] != NULL)
		{
			thread->filename = thread->global->fileArray[thread->fileOn];
			while(fscanf(thread->filename, "%1024s", hostnames) > 0)
					{
						while(done == 0)
						{
							pthread_mutex_lock(&thread->global->shared_array);
							if(q_full(thread->global->shared_array) == 0)
							{
								temp = malloc(MaxNameLength);
								strncpy(temp, hostnames, MaxNameLength);
								q_push(thread->global->shared_array, temp);
								pthread_mutex_unlock(&thread->global->queue_lock);
								done = 1;
							}
							else
							{
								pthread_mutex_unlock(&thread->global->queue_lock);
								done = 0;
							}
						}
						done = 0;
				}
			}
			thread->count += 1;
			thread->fileOn += 1;
	}
	pthread_mutex_unlock(&thread->global->service_lock);
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
	void* temp;
	while(q_empty(thread->global->shared_array) == 0 || *thread->global->running == 1)
	{
		if(q_empty(thread->global->shared_array) == 0)
		{
			pthread_mutex_lock(&thread->global->queue_lock);
			temp = q_pop(thread->global->shared_array);
			pthread_mutex_unlock(&thread->global->queue_lock);
			strncpy(hostnames, temp, MaxNameLength);
			pthread_mutex_lock(&thread->global->result_lock);
			fprintf(thread->results, "%s,", hostnames);
			if(dnslookup(hostnames, ipaddress, len) == UTIL_FAILURE)
			{
				fprintf(thread->results, "\n");
			}
			else
			{
				fprintf(thread->results, "%s\n", ipaddress);
			}
		}
		pthread_mutex_unlock(&thread->global->result_lock);
	}
	free(temp);
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
	FILE* service;
	FILE* result;
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
	if(atoi(argv[1]) > MaxRequesterThreads)
	{
		//fprintf(stderr, "You requested too many requester threads!\n");
		//fprintf(stderr, "You've asked for %d requester threads\n", atoi(argv[1]));
		//fprintf(stderr, "Setting number of Requester threads to max of %d\n", MaxRequesterThreads);
		request = MaxRequesterThreads;
	}
	if(atoi(argv[2]) > MaxResolverThreads)
	{
		//fprintf(stderr, "You requested too many resolver threads!\n");
		//fprintf(stderr, "You've asked for %d resolver threads\n", atoi(argv[2]));
		//fprintf(stderr, "Setting number of Resolver threads to max of %d\n", MaxResolverThreads);
		resolve = MaxResolverThreads;
	}
	if(queue_init(&shared_array, MaxNameLength) == -1) fprintf(stderr, "Queue init failed");

	for(int i=0; i<number_files; i++){
		in_files[i] = fopen(argv[5+i], "r");
	}

	//pthread initialization
	pthread_mutex_init(&global.queue_lock, NULL);
	pthread_mutex_init(&global.service_lock, NULL);
	pthread_mutex_init(&global.result_lock, NULL);
	//global base initialization
	global.shared_array = &shared_array;
	global.file_count   = number_files;
	global.running      = &running;
	global.fileArray    = in_file;
	//creating requster struct
	error = 0;
	for(int i = 0; i < request_count; i++){
		request_t[i].global   = &global;
		request_t[i].id       = i;
		request_t[i].count    = 0;
		request_t[i].serviced = service_file;
		request_t[i].filename = in_file[i];
		error = pthread_create(&req_threads[i], NULL, requester, &request_t[i]);
		if(!error){
			fprintf(stderr, "Failed to create pthread\n");
		}
	}
	error = 0;
	for(int i = 0; i < resolve_count; i++){
		resolve_t[i].global  = &global;
		resolve_t[i].results = result_file;
		error = pthread_create(&(res_threads[i]), NULL, resolver, &(resolve_t[i]));
		if(!error){
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
	q_clean(shared_array);

	gettimeofday(&end, NULL);
	long seconds = (end.tv_sec - start.tv_sec);
	long micros = ((seconds * 1000000) + end.tv_usec) - (start.tv_usec);
	printf("Multi-Lookup took %ld seconds and %ld micros to complete\n", seconds, micros);

	return 0;
}
