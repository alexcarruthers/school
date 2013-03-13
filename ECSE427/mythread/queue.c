#include <stdio.h>
#include <stdlib.h>
typedef struct queue_element{
	int num;
	struct queue_element* next;
}queue_element;

typedef struct queue{
	struct queue_element* head;
	struct queue_element* tail;
	int length;
}queue;

queue* create_queue(){
	queue* new_queue = malloc(sizeof(queue));
	new_queue->head = NULL;
	new_queue->tail = NULL;
	new_queue->length = 0;
	return new_queue;
}

void add_element(queue* my_queue, int num){
	queue_element* new_element = malloc(sizeof(queue_element));
	new_element->num = num;
	new_element->next = NULL;
	if(my_queue->head == NULL && my_queue->tail == NULL){
		my_queue->head = new_element;
		my_queue->tail = new_element;
	}
	else{
		my_queue->tail->next = new_element;
		my_queue->tail = new_element;
	}
	my_queue->length++;
}

queue_element* get_element(queue* my_queue){
	queue_element* element = my_queue->head;
	if(my_queue->head == NULL && my_queue->tail == NULL){
		return NULL;
	}
	else if(my_queue->head == my_queue->tail){
		my_queue->head == NULL;
		my_queue->tail == NULL;
		my_queue->length--;
		return element;
	}
	else{
		my_queue->head = element->next;
		my_queue->length--;
		return element;
	}
}

void print_queue(queue* my_queue){
	queue_element* element;
	for(element = my_queue->head; element; element = element->next){
		printf("%d ", element->num);
	}
	printf("\n");
}

int queue_length(queue* my_queue){
	return my_queue->length;
}
