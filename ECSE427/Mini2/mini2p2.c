#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "queue.c"

queue* consumer_producer_queue;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* consumer(int* arg){
	int max = *arg;
	int i = 0;
	queue_element* element;
	while(i<max){
		pthread_mutex_lock(&mutex);
		element = get_element(consumer_producer_queue);
		pthread_mutex_unlock(&mutex);
		if(element){
			i++;
		}
	}
	printf("%d Items Consumed\n", i);
	pthread_exit(0);
}

void* producer(int *arg){
	int i = *arg;
	while(i > 0){
		pthread_mutex_lock(&mutex);
		add_element(consumer_producer_queue, i);
		pthread_mutex_unlock(&mutex);
		i--;
	}
	pthread_exit(0);
}

int main(){
	int num_producers = 10;
	int N = 10;
	int tenN = num_producers*N;
	struct timeval begtime, endtime;
	gettimeofday(&begtime, NULL);
	consumer_producer_queue = create_queue();
	pthread_t prod[num_producers];
	pthread_t cons;
	
	int i;
	for(i=0; i<10; i++){
		pthread_create(&prod[i],NULL,producer,&N);
	}
	pthread_create(&cons,NULL,consumer,&tenN);
	pthread_join(cons,NULL);
	for(i=0;i<10;i++){
		pthread_join(prod[0],NULL);
	}
	gettimeofday(&endtime, NULL);
	printf("total time: %lu microseconds\n",endtime.tv_usec - begtime.tv_usec);
}