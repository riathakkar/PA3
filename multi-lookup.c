#include<util.h>
#define MaxInputFiles 10
#define MaxResolverThreads 10
#define MaxRequesterThreads 5
#define MaxNameLength 1025
#define MaxIpLength INET6_ADDRSTRLEN

int main(int argc, char *argv[]){
	int num_names = argc  - 5; //5 is the base argument count
	int requester_count = atoi(argv[1]);
	int resolver_count = atoi(arv[2]);

	int thread_running = 1;

	//argv[3] - results out file
	//argv[4] - service out file
	FILE* resultfile = NULL;
	FILE* servicefile = NULL;
	FILE* in_files[num_names];
	//create request/resolve threads
	pthread_t request_threads[requester_count];
	pthread_t resolve_threads[resolver_count];

	//create mutexes
	pthread_mutex_t shared_space_lock;
	pthread_mutex_t file_lock;
	pthread_mutex_t output_lock;

	queue shared_space;

	if(q_init(&shared_space, 100) == -1){
		fprintf(stderr, "creating shared queue error\n");
		return -1;
	}

	pthread_mutex_init(&shared_space_lock, NULL);
	pthread_mutex_init(&file_lock, NULL);
	pthread_mutex_init(&output_lock, NULL);
	
	struct req_thread_info req_t_info;
	struct res_thread_info res_t_info;
	
	resultfile = fopen(argv[3], "w");
	servicefile = fopen(argv[4], "w");
	//add error handling
	
	for(int i = 0; i < num_names; i++){
		in_files[i] = fopen(argv[i+5], "r");
		if(!in_files[i]) fprintf(stderr,"Error opening file");
	}

	for(int i = 0; i < num_names; i++){
		FILE* curFile=inputFiles[i];
		req_t_info[i].queue_lock = &shared_space_lock;
		req_t_info[i].file_name = curFile;
		req_t_info[i].queue = &shared_space;
		req_t_info[i].file_lock = &file_lock;
		req_t_info[i].service_name = servicefile;
		//add error handling
		req_t_info[i].num_on = i;
		req_t = pthread_create(&request_threads[i], NULL, requester, &req_t_info[i]);
	}
	for(int i = 0; i < resolver_count; i++){
		res_t_info[i].queue_lock = &shared_space_lock;
		res_t_info[i].file_name = resultfile;
		res_t_info[i].file_lock = &output_lock;
		res_t_info[i].queue = &shared_space;
		res_t_info[i].running = &thread_running;
		pthread_create(&resolve_threads[i], NULL, resolver, &res_t_info[i]);
		//add error handling
	}
	for(int i = 0; i < infiles; i++){
		pthread_join(request_threads[i], NULL)
	}
	
	thread_running=0;
	//Now we wait for the output threads to finish running
	for(int i=0;i<resolver_count;i++){
        	pthread_join(resolve_threads[i],NULL);
    	}
	
	fclose(resultfile);
	fclose(servicefile);
	q_clean(&shared_space);
	fcloseall();

	pthread_mutex_destroy(&shared_space_lock);
	pthread_mutex_destroy(&file_lock);
	pthread_mutex_destroy(&output_lock);
	
	
	return 0;
}

void* requester(void *p){
	char hostnames[MAX_NAME_LENGTH];
	struct* req_thread_info thread = p;
	
	int done = 0;
	while(thread->num_on <
	
}


