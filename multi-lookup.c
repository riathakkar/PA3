  
#include "util.h"
#include "multi-lookup.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#define MaxInputFiles 10
#define MaxResolverThreads 10
#define MaxRequesterThreads 5
#define MaxNameLength 1025
#define MaxIpLength INET6_ADDRSTRLEN
#define MaxStrSize 50
int main(int argc, char *argv[]){
	struct timeval start;
	struct timeval end;

	gettimeofday(&start, NULL);

	if(argc >= 6){
	  struct Globals global;
	  //add mutex failure case here
	  //initialize mutex
	  int request_count = atoi(argv[1]);
	  int resolve_count = atoi(argv[2]);

	  if(request_count == 0 || resolve_count == 0){
	    fprintf(stderr, "Invalid argument used. Aborting");
	    return -1;
	  }

	  if(resolve_count > MaxResolverThreads){
	    resolve_count = MaxResolverThreads;
	  }
	  if(request_count > MaxRequesterThreads){
	    request_count = MaxRequesterThreads;
	  }
	  for(int i = 0; i < MaxInputFiles; i++){
	    pthread_mutex_init(&global.input_lock[i], NULL);
	  }
	  char *results = argv[3];
	  char *service = argv[4];
	  FILE *request_file;
	  FILE *resolve_file;

	  request_file = fopen(results, "w");
	  resolve_file = fopen(service, "w");

	  int input_file_count = argc - 5;
	  global.file_count = input_file_count;
	  global.fileOn = 0;
	  global.serviced = request_file;
	  global.results = resolve_file;
	  pthread_mutex_init(&global.queue_lock, NULL);
	  pthread_mutex_init(&global.service_lock, NULL);
	  pthread_mutex_init(&global.result_lock, NULL);

	  pthread_cond_init(&global.cond_request, NULL);
	  pthread_cond_init(&global.cond_resolve, NULL);

      	  if(q_init(&global.shared_array, MaxNameLength) == -1) fprintf(stderr, "Queue init failed");
	  //	  fprintf(stdout, input_file_count);
	  for(int i = 0; i < input_file_count; i++){
	    global.fileArray[i]=fopen(argv[i+5], "r");
	  }

	  for(int i = 0; i < MaxInputFiles; i++){
	    global.file_status[i] = 0;
	  }

	  if(request_file && resolve_file){
      	    for(int i = 0; i < request_count; i++){
	      pthread_create(&global.req_threads[i], NULL, requester, &global);
	    }
	    //for(int i = 0; i < resolve_count; i++){
	    //  pthread_create(&global.res_threads[i], NULL, resolver, &global);
	    //}

	    for(int i = 0; i < request_count; i++){
	      pthread_join(global.req_threads[i], NULL);
	    }
	    //for(int i = 0; i < resolve_count; i++){
	    // pthread_join(global.res_threads[i], NULL);
	    //}
	    gettimeofday(&end, NULL);
	    printf("Program ran in %ld microseconds\n", (((end.tv_sec * 1000000) + end.tv_usec) - ((start.tv_sec * 1000000) + start.tv_usec)));
	  } else {
	    fprintf(stderr, "Invalid output file");      
	  }
	}
	else{
	  fprintf(stderr, "Not enough arguments. Need atleast 6.\n");
	}
	return 0;
}

void *requester(void *p){
  struct Globals *globals = p;
  char *temp;
  char name[MaxStrSize];
  int counter = 0;

  pid_t threadid = syscall(SYS_gettid);
  fprintf(stdout, "in requester");
  pthread_mutex_lock(&(globals->queue_lock));
  int i = globals->fileOn;
  globals->fileOn++;
  pthread_mutex_unlock(&globals->queue_lock);

  while(i < globals->file_count){
    if(globals->file_status[i]) continue;
    counter += 1;
    while(1){
      pthread_mutex_lock(&globals->input_lock[i]);
      while(q_full(&globals->shared_array)) pthread_cond_wait(&globals->cond_request, &globals->input_lock[i]);
      if(fgets(name, sizeof name, globals->fileArray[i])){
	pthread_cond_signal(&globals->cond_resolve);
	pthread_mutex_unlock(&globals->input_lock[i]);

	name[strlen(name) - 1] = '\0';

	pthread_mutex_lock(&globals->queue_lock);
	temp = malloc(MaxNameLength);
	strncpy(temp, name, MaxNameLength);
	q_push(&globals->shared_array, temp);
	free(temp);
	pthread_mutex_unlock(&globals->queue_lock);
      } else {
	pthread_mutex_unlock(&globals->input_lock[i]);
	globals->file_status[i] = 1;
	break;
      }
    }
    i++;
  }
  pthread_mutex_lock(&globals->service_lock);
  fprintf(globals->serviced, "Thread <%d> serviced %d files.\n", threadid, counter);
  pthread_mutex_unlock(&globals->service_lock);
  return NULL;
}
void *resolver(void *p){
  struct Globals *global = p;
  char ipaddress[MaxIpLength];
  char hostnames[MaxStrSize];
  char *temp;
  pthread_mutex_lock(&globals->queue_lock);
  while(!all_done(globals->file_status, globals->file_count) || !queue_is_empty(globals->shared_array){
      while(q_empty(globals->shared_array)) pthread_cond_wait(&globals->cond_resolve, &globals->queue_lock);

      temp = q_pop(globals->shared_array);
      pthread_cond_signal(&globals->cond_request);
      pthread_mutex_unlock(&globals->queue_lock);

      if(dnslookup(temp, ipaddress, MaxIpLength) == UTIL_FAILURE){
	printf("%s couldn't be resolved.\n", temp);
	pthread_mutex_lock(&globals->result_lock);
	fprintf(globals->results, "%s,\n", temp);
	pthread_mutex_unlock(&globals->result_lock);
      } else {
	pthread_mutex_lock(&globals->result_lock);
	fprintf(globals->results, "%s, \n", temp, ipaddress);
	pthread_mutex_unlock(&globals->result_lock);
      }
      pthread_mutex_unlock(&globals->queue_lock);
    }
    return 0;
}
char all_done(char* files, int len) {
	char res = 1;
	for (int i = 0; i < len; i++) {
		res &= files[i];
	}

	return res;
}
/*void *junk(){
 while(i < globals->file_count){
    if(globals->file_status[i]) continue;
    counter += 1;
    while(1){
      pthread_mutex_lock(&globals->input_lock[i]);
      while(q_full(&globals->shared_array)) pthread_cond_wait(&globals->cond_request, &globals->input_lock[i]);
      if(fgets(name, sizeof name, f)){
	pthread_cond_signal(&globals->cond_resolve);
	pthread_mutex_unlock(&globals->input_lock[i]);

	name[strlen(name) - 1] = '\0';

	pthread_mutex_lock(&globals->queue_lock);
	temp = malloc(MaxNameLength);
	strncpy(temp, name, MaxNameLength);
	q_push(&globals->shared_array, temp);
	free(temp);
	pthread_mutex_unlock(&globals->queue_lock);
      } else {
	pthread_mutex_unlock(&globals->input_lock[i]);
	globals->file_status[i] = 1;
	break;
      }
    }
    i++;
  }
  pthread_mutex_lock(&globals->service_lock);
  fprintf(globals->serviced, "Thread <%d> serviced %d files.\n", threadid, counter);
  pthread_mutex_unlock(&globals->service_lock);
}*/


