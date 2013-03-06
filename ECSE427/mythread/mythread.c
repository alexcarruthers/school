#include <slack/std.h>
#include <slack/list.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>

#define THREAD_NAME_LENGTH 100
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef enum {EXIT, RUNNABLE, RUNNING, BLOCKED} THREAD_STATE;

typedef struct {
	ucontext_t context;
	int thread_id;
	THREAD_STATE state;
	char thread_name[THREAD_NAME_LENGTH];
	int cpu_run_time;
	char *stack;
}mythread_control_block;

typedef struct{
	int count;
	int initial_count;
	List *waiting_threads;
}semaphore_t;

List *runqueue;
semaphore_t *sems[100]; //have maximum 100 semaphores
mythread_control_block *threads[100]; //have maximum 100 threads
static ucontext_t uctxt_main;
int next_thread;
int next_semaphore;
int currently_running_thread;

int mythread_init(){
	runqueue = list_create(NULL);
	next_semaphore = 0;
	next_thread = 0;
	currently_running_thread = -1;
	int i;
	for (i=0; i<100; i++){
		sems[i] = NULL;
		threads[i] = NULL;
	}
	return 0;
}

int mythread_create(char *threadname, void (*threadfunc)(), int stacksize){
	mythread_control_block *new_thread = malloc(sizeof(mythread_control_block));
	if (getcontext(&new_thread->context) == -1){
		handle_error("getcontext");
	}
	new_thread->stack = malloc(stacksize);
	new_thread->context.uc_stack.ss_sp = new_thread->stack;
	new_thread->context.uc_stack.ss_size = sizeof(new_thread->stack);
	new_thread->context.uc_link = &uctxt_main;
	new_thread->thread_id = next_thread;
	new_thread->cpu_run_time = 0;
	new_thread->state = RUNNABLE;
	makecontext(&new_thread->context, threadfunc, 0);
	strcpy(new_thread->thread_name, threadname);
	threads[next_thread] = new_thread;
	next_thread++;
	runqueue = list_append_int(runqueue, new_thread->thread_id);
	return new_thread->thread_id;
	
}
void mythread_exit(){
	
}
void runthreads();
void set_quantum_size(int quantum);

int create_semaphore(int value){
	semaphore_t *sem = malloc(sizeof(semaphore_t));
	sem->count = value;
	sem->waiting_threads = list_create(NULL);
	sems[next_semaphore] = sem;
	next_semaphore++;
	return next_semaphore - 1;
}

void semaphore_wait(int semaphore);
void semaphore_signal(int semaphore);
void destroy_semaphore(int semaphore);
void mythread_state();

void test(){
	printf("test\n");
}


int main(){
	mythread_init();
	int thread = mythread_create("test",test,8192);
	swapcontext(&uctxt_main, &threads[thread]->context);
	printf("yay");
	return 0;
}
