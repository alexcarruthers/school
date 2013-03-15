#include <signal.h>
#include <slack/std.h>
#include <slack/list.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <sys/time.h>
#include "mythread.h"

#define THREAD_NAME_LENGTH 100
#define THREADS_SEMAPHORES_MAX 100

//define what a thread and semaphore contain
typedef struct {
	ucontext_t context;
	int thread_id;
	THREAD_STATE state;
	char thread_name[THREAD_NAME_LENGTH];
	int cpu_run_time;
	long time_block_begin;
	void *stack;
}mythread_control_block;
typedef struct {
	int semaphore_id;
	int count;
	int initial_count;
	List *waiting_threads;
}semaphore_t;

//define required lists
static List *runqueue;
static semaphore_t sems[THREADS_SEMAPHORES_MAX]; //have maximum 100 semaphores
static mythread_control_block threads[THREADS_SEMAPHORES_MAX]; //have maximum 100 threads

//define main context
static ucontext_t uctxt_main;

//next available semaphores and lists
static int next_thread;
static int next_semaphore;

static int currently_running_thread;
static int quantum_size = 0;
struct itimerval tval;

//initiate mythread
int mythread_init(){
	//initiate values
	runqueue = list_create(NULL);
	next_semaphore = 0;
	next_thread = 0;
	currently_running_thread = -1;
	return 0;
}

//create a thread
int mythread_create(char *threadname, void (*threadfunc)(), int stacksize){
	if (next_thread == THREADS_SEMAPHORES_MAX) {
		perror("Max number of threads reached");
		return -1;
	}
	//get the context for the next available thread in the threads
	//table and set up the context
	getcontext(&threads[next_thread].context);
	threads[next_thread].stack = malloc(stacksize);
	threads[next_thread].context.uc_stack.ss_sp = threads[next_thread].stack;
	threads[next_thread].context.uc_stack.ss_size = stacksize;
	threads[next_thread].context.uc_stack.ss_flags = 0;
	threads[next_thread].context.uc_link = &uctxt_main;
	sigemptyset(&threads[next_thread].context.uc_sigmask);
	
	//initiate all other thread fields
	threads[next_thread].thread_id = next_thread;
	threads[next_thread].cpu_run_time = 0;
	threads[next_thread].state = RUNNABLE;
	strcpy(threads[next_thread].thread_name, threadname);
	
	//create the context
	makecontext(&threads[next_thread].context, threadfunc, 0);

	//add the thread to the runqueue
	runqueue = list_append_int(runqueue, threads[next_thread].thread_id);
	
	//increment the thread counter so that the next thread can be made
	next_thread++;
	
	return threads[next_thread-1].thread_id;
	
}

//marks the currently running thread as exited
void mythread_exit(){
	if (currently_running_thread == -1){
		handle_error("no threads running");
	}
	threads[currently_running_thread].state = EXIT;
	swapcontext(&threads[currently_running_thread].context,&uctxt_main);
	return;
}

//run all the threads in the runqueue
void runthreads(){
	//check for erroneous states
	if (quantum_size == 0){
		handle_error("quantum not set");
	}
	if (next_thread == 0){
		handle_error("No threads initialized");
	}
	
	//define what function to run once SIGALRM is raised
	sigset(SIGALRM,&SIGALRM_handler);
	
	//set up and start the timer 
	tval.it_interval.tv_sec = 0;
	tval.it_interval.tv_usec = quantum_size;
	tval.it_value = tval.it_interval;
	setitimer(ITIMER_REAL, &tval, 0);
	
	//iterate over all threads
	while(!list_empty(runqueue)){
		//get the next thread and mark it running
		currently_running_thread = list_shift_int(runqueue);
		threads[currently_running_thread].state = RUNNING;
		//mark when this thread is starting it's time on the cpu
		start_timer(currently_running_thread);
		//switch to the desired thread
		swapcontext(&uctxt_main, &threads[currently_running_thread].context);
		//mark the end of the thread's time on the cpu and save the elapsed time
		stop_timer(currently_running_thread);
		//add thread to the end of the runqueue if it hasnt finished or isn't blocked
		if(threads[currently_running_thread].state == RUNNABLE || threads[currently_running_thread].state == RUNNING){
			runqueue = list_append_int(runqueue, currently_running_thread);
		}
	}
}

//function that runs when SIGALRM is signaled
void SIGALRM_handler(){
	//switch back to the runthreads function
	if(!list_empty(runqueue)){
		swapcontext(&threads[currently_running_thread].context,&uctxt_main);
	}
}

//set the quantum size
void set_quantum_size(int quantum){
	quantum_size = quantum;
}

//create a semaphore
int create_semaphore(int value){
	if (next_semaphore == THREADS_SEMAPHORES_MAX) {
		perror("Max number of semaphores reached");
		return -1;
	}
	//initialize all the semaphore values
	sems[next_semaphore].count = value;
	sems[next_semaphore].initial_count = value;
	sems[next_semaphore].waiting_threads = list_create(NULL);
	sems[next_semaphore].semaphore_id = next_semaphore;
	next_semaphore++;
	return next_semaphore - 1;
}

//waits on a semaphore
void semaphore_wait(int semaphore){
	//block all SIGALRMs
	sigset_t blocker;
	sigemptyset(&blocker);
	sigaddset(&blocker, SIGALRM);
	sigprocmask(SIG_BLOCK, &blocker, NULL);
	
	//decrement the semaphore
	sems[semaphore].count--;
	//if the thread has to wait, append the thread to the list of waiting semaphores
	if (sems[semaphore].count < 0){
		sems[semaphore].waiting_threads = list_append_int(sems[semaphore].waiting_threads, currently_running_thread);
		threads[currently_running_thread].state = BLOCKED;
	}
	
	//unblock the signals and switch to the next thread
	sigprocmask(SIG_UNBLOCK, &blocker, NULL);
	SIGALRM_handler();
}

//signal a semaphore
void semaphore_signal(int semaphore){
	//block signals
	sigset_t blocker;
	sigemptyset(&blocker);
	sigaddset(&blocker, SIGALRM);
	sigprocmask(SIG_BLOCK, &blocker, NULL);
	
	//increment the semaphore
	sems[semaphore].count++;
	
	//if the semaphore is low, get the next waiting thread and append it to the runqueue
	if (sems[semaphore].count <= 0){
		int to_run = list_shift_int(sems[semaphore].waiting_threads);
		threads[to_run].state = RUNNABLE;
		runqueue = list_append_int(runqueue, to_run);
	}
	
	//unblock the signals and run the thread switcher
	sigprocmask(SIG_UNBLOCK, &blocker, NULL);
	SIGALRM_handler();
}

//destroy a semaphore
void destroy_semaphore(int semaphore){
	if (!list_empty(sems[semaphore].waiting_threads)) {
		perror("Can't destroy semaphore while threads waiting on it.");
	}
	else if (sems[semaphore].count != sems[semaphore].initial_count){
		perror("Something went wrong, initial value not equal to current value");
	}
	return;
}

//marks the start of a threads current run on the cpu
void start_timer(int thread){
	struct timeval time;
	gettimeofday(&time, NULL);
	threads[thread].time_block_begin = (time.tv_sec * 1000000 + time.tv_usec);
}

//marks the end of a threads run on the cpu saves the amount of time
void stop_timer(int thread){
	struct timeval time;
	gettimeofday(&time, NULL);
	threads[thread].cpu_run_time += (time.tv_sec * 1000000 + time.tv_usec) - threads[thread].time_block_begin;
}

//translates the THREAD_STATE enum into strings
const char* getStateString(THREAD_STATE state){
	switch(state){
		case EXIT: return "EXIT"; break;
		case RUNNABLE: return "RUNNABLE"; break;
		case RUNNING: return "RUNNING"; break;
		case BLOCKED: return "BLOCKED"; break;
	}
	return "";
}

//prints out the state of all the threads
void mythread_state(){
	printf("Thread ID\tThread Name\tThread State\tCPU time (in microseconds)\n");
	printf("--------------------------------------------------------------------------\n");
	int i;
	for (i=0;i<next_thread;i++){
		printf("%d\t\t%s\t%s\t\t%d\n",threads[i].thread_id,threads[i].thread_name, getStateString(threads[i].state), threads[i].cpu_run_time);
	}
}

//handles error messages and quits
void handle_error(char *msg) {
	perror(msg); 
	exit(EXIT_FAILURE); 
}
