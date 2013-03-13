#include <slack/std.h>
//#include <slack/list.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include "queue.c"

#define THREAD_NAME_LENGTH 100
#define NUMBER_THREADS_SEMAPHORES 100
void handle_error(char* msg){
	perror(msg);
	exit(EXIT_FAILURE); 
}

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
	queue *waiting_threads;
}semaphore_t;

queue *runqueue;
semaphore_t *sems[NUMBER_THREADS_SEMAPHORES]; //have maximum 100 semaphores
mythread_control_block *threads[NUMBER_THREADS_SEMAPHORES]; //have maximum 100 threads
ucontext_t uctxt_main;

int next_thread;
int next_semaphore;
int currently_running_thread;
int threads_running;

struct sigaction act;
struct itimerval tval;

ucontext_t sig_context;
void *sig_stack;
int sig_stack_size = 4096;

/*void print(List* l){
	printf("LIST: ");
	while(list_has_next(l)){
		printf("%d ", list_next_int(l));
	}
	printf("\n");
}*/


void runthreads(){
	currently_running_thread = get_element(runqueue)->num;
	printf("RUNNING THREAD: %d\n", currently_running_thread);
	printf("LIST: ");
	print_queue(runqueue);
	setcontext(&threads[currently_running_thread]->context);
}

void scheduler(){
	int oldthread = currently_running_thread;
	currently_running_thread = get_element(runqueue)->num;
	add_element(runqueue, oldthread);
	printf("RUNNING THREAD: %d\n", currently_running_thread);
	printf("LIST: ");
	print_queue(runqueue);
	setcontext(&threads[currently_running_thread]->context);
}

void timer_handler(){
	//swapcontext(&threads[currently_running_thread]->context,&uctxt_main);
	printf("HERE?\n");
	getcontext(&sig_context);
	sig_context.uc_stack.ss_sp = sig_stack;
	sig_context.uc_stack.ss_size = sig_stack_size;
	sig_context.uc_stack.ss_flags = 0;
	sigemptyset(&sig_context.uc_sigmask);
	makecontext(&sig_context, scheduler, 0);
	swapcontext(&threads[currently_running_thread]->context,&sig_context);
}

void init_signals(){
	act.sa_sigaction = timer_handler;
	sigemptyset(&act.sa_mask);
//	act.sa_flags = SA_RESTART | SA_SIGINFO;
	if(sigaction(SIGALRM, &act, NULL) != 0) handle_error("Signal Handler");
	
}

int mythread_init(){
	runqueue = create_queue();
	next_semaphore = 0;
	next_thread = 0;
	currently_running_thread = -1;
	int i;
	for (i=0; i<NUMBER_THREADS_SEMAPHORES; i++){
		sems[i] = malloc(sizeof(semaphore_t));
		threads[i] = malloc(sizeof(mythread_control_block));
	}
	
	sig_stack = malloc(sig_stack_size);
	
	tval.it_interval.tv_sec = 0;
	tval.it_interval.tv_usec = 150;
	tval.it_value = tval.it_interval;
	init_signals();
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
	sigemptyset(&threads[next_thread]->context.uc_sigmask);
	makecontext(&threads[next_thread]->context, threadfunc, 0);
	strcpy(threads[next_thread]->thread_name, threadname);
	add_element(runqueue, threads[next_thread]->thread_id);
	next_thread++;
	return threads[next_thread-1]->thread_id;
	
}
void mythread_exit(){
	if (currently_running_thread == -1){
		handle_error("no threads running");
	}
	threads[currently_running_thread]->state = EXIT;
}

void set_quantum_size(int quantum);





int create_semaphore(int value){
	sems[next_semaphore]->count = value;
	sems[next_semaphore]->initial_count = value;
	sems[next_semaphore]->waiting_threads = create_queue();
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

void test1(){
	int i;
	printf("test1\n");
	for (i=1000; i<2000; i++){
		//printf("%d", i);
	}
}
void test2(){
	int i;
	printf("test2\n");
	for (i=2000; i<3000; i++){
		//printf("%d", i);
	}
}
void test3(){
	int i;
	printf("test3\n");
	for (i=3000; i<4000; i++){
		//printf("%d", i);
	}
}
void test4(){
	int i;
	printf("test4\n");
	for (i=4000; i<5000; i++){
		//printf("%d", i);
	}
}


int main(){
	mythread_init();
	
	mythread_create("test1",test1,8192);
	mythread_create("test2",test2,8192);
	mythread_create("test3",test3,8192);
	mythread_create("test4",test4,8192);
	mythread_state();
	runthreads();
	return 0;
}
