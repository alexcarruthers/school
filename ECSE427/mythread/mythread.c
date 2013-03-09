#include <slack/std.h>
#include <slack/list.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#define THREAD_NAME_LENGTH 100
#define NUMBER_THREADS_SEMAPHORES 100
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef enum THREAD_STATE{EXIT, RUNNABLE, RUNNING, BLOCKED} THREAD_STATE;

const char* getStateString(THREAD_STATE state){
	switch(state){
		case EXIT: return "EXIT"; break;
		case RUNNABLE: return "RUNNABLE"; break;
		case RUNNING: return "RUNNING"; break;
		case BLOCKED: return "BLOCKED"; break;
	}
	return "";
}

typedef struct mythread_control_block{
	ucontext_t context;
	int thread_id;
	THREAD_STATE state;
	char thread_name[THREAD_NAME_LENGTH];
	int cpu_run_time;
	char *stack;
}mythread_control_block;

typedef struct semaphore_t{
	int semaphore_id;
	int count;
	int initial_count;
	List *waiting_threads;
}semaphore_t;

List *runqueue;
semaphore_t *sems[NUMBER_THREADS_SEMAPHORES]; //have maximum 100 semaphores
mythread_control_block *threads[NUMBER_THREADS_SEMAPHORES]; //have maximum 100 threads
static ucontext_t uctxt_main;

int next_thread;
int next_semaphore;
int currently_running_thread;
int threads_running;

struct sigaction SIGALRM_action;
struct itimerval tval;


void SIGALRM_handler(){
	swapcontext(&threads[currently_running_thread]->context,&uctxt_main);
}

void print(List* l){
	printf("LIST: ");
	while(list_has_next(l)){
		printf("%d ", list_next_int(l));
	}
	printf("\n");
}

int mythread_init(){
	runqueue = list_create(NULL);
	next_semaphore = 0;
	next_thread = 0;
	currently_running_thread = -1;
	int i;
	for (i=0; i<NUMBER_THREADS_SEMAPHORES; i++){
		sems[i] = malloc(sizeof(semaphore_t));
		threads[i] = malloc(sizeof(mythread_control_block));
	}
	
	tval.it_interval.tv_sec = 0;
	tval.it_interval.tv_usec = 100;
	tval.it_value.tv_sec = 0;
	tval.it_value.tv_usec = 100;
	SIGALRM_action.sa_handler = SIGALRM_handler;
	SIGALRM_action.sa_flags = 0;
	sigset(SIGALRM,SIGALRM_handler);
	return 0;
}

int mythread_create(char *threadname, void (*threadfunc)(), int stacksize){
	if (getcontext(&threads[next_thread]->context) == -1){
		handle_error("getcontext");
	}
	threads[next_thread]->stack = malloc(stacksize);
	threads[next_thread]->context.uc_stack.ss_sp = threads[next_thread]->stack;
	threads[next_thread]->context.uc_stack.ss_size = sizeof(threads[next_thread]->stack);
	threads[next_thread]->context.uc_link = &uctxt_main;
	threads[next_thread]->thread_id = next_thread;
	threads[next_thread]->cpu_run_time = 0;
	threads[next_thread]->state = RUNNABLE;
	makecontext(&threads[next_thread]->context, threadfunc, 0);
	strcpy(threads[next_thread]->thread_name, threadname);
	runqueue = list_append_int(runqueue, threads[next_thread]->thread_id);
	next_thread++;
	return threads[next_thread-1]->thread_id;
	
}
void mythread_exit(){
	if (currently_running_thread == -1){
		handle_error("no threads running");
	}
	threads[currently_running_thread]->state = EXIT;
}
void runthreads(){
	setitimer(ITIMER_REAL, &tval, 0);
	while(list_length(runqueue) != 0){
		currently_running_thread = list_shift_int(runqueue);
		print(runqueue);
		if(currently_running_thread == -1) handle_error("Get item");
		printf("Running thread: %d\n", currently_running_thread);
		swapcontext(&uctxt_main, &threads[currently_running_thread]->context);
		if(threads[currently_running_thread]->state != EXIT){
			runqueue = list_append_int(runqueue, currently_running_thread);
		}
	}
}
void set_quantum_size(int quantum);





int create_semaphore(int value){
	sems[next_semaphore]->count = value;
	sems[next_semaphore]->initial_count = value;
	sems[next_semaphore]->waiting_threads = list_create(NULL);
	sems[next_semaphore]->semaphore_id = next_semaphore;
	next_semaphore++;
	return next_semaphore - 1;
}

void semaphore_wait(int semaphore);
void semaphore_signal(int semaphore);
void destroy_semaphore(int semaphore);
void mythread_state(){
	printf("Thread ID\tThread Name\tThread State\tCPU time\n");
	printf("--------------------------------------------------\n");
	int i;
	for (i=0;i<next_thread;i++){
		printf("%d\t\t%s\t\t%s\t%d\n",threads[i]->thread_id,threads[i]->thread_name, getStateString(threads[i]->state), threads[i]->cpu_run_time);
	}
}

void test(){
	int i;
	printf("test\n");
	for (i=1000; i<2000; i++){
		//printf("%d\n", i);
	}
}
void test2(){
	int i;
	printf("test2\n");
	for (i=2000; i<3000; i++){
		//printf("%d\n", i);
	}
}
void test3(){
	int i;
	printf("test3\n");
	for (i=3000; i<4000; i++){
		//printf("%d\n", i);
	}
}
void test4(){
	int i;
	printf("test4\n");
	for (i=4000; i<5000; i++){
		//printf("%d\n", i);
	}
}


int main(){
	mythread_init();
	
	mythread_create("test",test,819);
	mythread_create("test2",test2,819);
	mythread_create("test3",test3,819);
	mythread_create("test4",test4,819);
	mythread_state();
	runthreads();
	return 0;
}
