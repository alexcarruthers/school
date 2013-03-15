#ifndef __MY_THREAD_H__
#define __MY_THREAD_H__

#include <signal.h>
#include <slack/std.h>
#include <slack/list.h>
#include <stdio.h>
#include <ucontext.h>
#include <string.h>
#include <sys/time.h>

#define THREAD_NAME_LENGTH 100
#define THREADS_SEMAPHORES_MAX 100

//enum to describe state of the thread
typedef enum {EXIT, RUNNABLE, RUNNING, BLOCKED}THREAD_STATE;

//define what a thread and semaphore contain
typedef struct mythread_control_block;
typedef struct semaphore_t;

//initiate mythread
int mythread_init();

//create a thread
int mythread_create(char *threadname, void (*threadfunc)(), int stacksize);

//marks the currently running thread as exited
void mythread_exit();

//run all the threads in the runqueue
void runthreads();

//function that runs when SIGALRM is signaled
void SIGALRM_handler();

//set the quantum size
void set_quantum_size(int quantum);

//create a semaphore
int create_semaphore(int value);

//waits on a semaphore
void semaphore_wait(int semaphore);

//signal a semaphore
void semaphore_signal(int semaphore);

//destroy a semaphore
void destroy_semaphore(int semaphore);

//marks the start of a threads current run on the cpu
void start_timer(int thread);

//marks the end of a threads run on the cpu saves the amount of time
void stop_timer(int thread);

//translates the THREAD_STATE enum into strings
const char* getStateString(THREAD_STATE state);

//prints out the state of all the threads
void mythread_state();

//handles error messages and quits
void handle_error(msg);
#endif
