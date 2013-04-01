#include <unistd.h>
#include <stdio.h>
#include <string.h>

typedef int policy;
#define FIRST_FIT 1
#define BEST_FIT 2

typedef int bool;
#define true 1
#define false 0

bool init = false;
void* sbrk_start;
void* first_free = NULL;
int numfree = 0;

//	In case my_mallopt is never called, assuming
//	first-fit as default
policy alloc_policy = FIRST_FIT;

static void* create_memblock(int size){
	int* blocklen;
	void* data;
	void * memblock = sbrk(size + sizeof(int));
	blocklen = memblock;
	*blocklen = size;
	data = memblock + sizeof(int);
	return data;
}

//if the memblock we need to unfree is the first in the list of free blocks, use this function
static void* unfree_first_memblock(int size){
	//Get all parts of the first free memory block
	int* first_free_len = first_free - sizeof(int);
	int** first_free_prevptr = first_free;
	int** first_free_nextptr = first_free + sizeof(int*);
	
	void* nextfreememblock = *(first_free_nextptr);
	
	//Get all parts of the next free memory block
	int* nextfreememblock_len = nextfreememblock - sizeof(int);
	int** nextfreememblock_prevptr = nextfreememblock;
	int** nextfreememblock_nextptr = nextfreememblock + sizeof(int*);
	
	if (*first_free_len - size < (int)(sizeof(int) + 2 * sizeof(int*))){
		//we can't split as the second block would be too small to hold the pointers and length
		first_free = nextfreememblock;
		*nextfreememblock_prevptr = NULL;
		return first_free_len + sizeof(int);
	}
	
	//we have to split the memblock and put the second half in the free list
	void* newmemblock = first_free_len + size + sizeof(int);
	int* newmemblock_len = newmemblock - sizeof(int);
	int** newmemblock_prevptr = newmemblock;
	int** newmemblock_nextptr = newmemblock + sizeof(int*);
	
	void* old_first_free = first_free;
	first_free = newmemblock;
	*newmemblock_len = *first_free_len - size - sizeof(int);
	*newmemblock_prevptr = NULL;
	*newmemblock_nextptr = *first_free_nextptr;
	return old_first_free;
}

//if the memblock we need to unfree is the last in the list of free blocks, use this function
static void* unfree_last_memblock(void* memblock, int size){
	//Get all parts of the last free memory block
	int* memblock_len = memblock - sizeof(int);
	int** memblock_prevptr = memblock;
	int** memblock_nextptr = memblock + sizeof(int*);
	
	void* prevfreememblock = *(memblock_prevptr);
	
	//Get all parts of the previous free memory block
	int* prevfreememblock_len = prevfreememblock - sizeof(int);
	int** prevfreememblock_prevptr = prevfreememblock;
	int** prevfreememblock_nextptr = prevfreememblock + sizeof(int*);
	
	if (*memblock_len - size < (int)(sizeof(int) + 2 * sizeof(int*))){
		//we can't split as the second block would be too small to hold the pointers and length
		prevfreememblock_nextptr = NULL;
		return memblock;
	}
	
	//we have to split the memblock and return the second half
	void* newmemblock = memblock + *memblock_len - size;
	int* newmemblock_len = newmemblock - sizeof(int);
	*newmemblock_len = size;
	*memblock_len = *memblock_len - size - sizeof(int);
	
	return newmemblock;
}

static void* unfree_middle_memblock

extern char *my_malloc_error;
void *my_malloc(int size){
	//need to make sure when freeing the 
	if (size < 2 * (int)sizeof(void*)){
		size = 2 * sizeof(void*);
	}
	
	if (!init){
		init = true;
		void* memblock = create_memblock(size);
		sbrk_start = memblock - sizeof(int);
		return memblock;
	}
	if (alloc_policy == FIRST_FIT){
		puts("test");
		void* memblock = sbrk_start;
		void* heap_end = sbrk(0);
		while (memblock < heap_end){
			//if (*memblock - sizeof(int) > 
		}
	}
	
	return NULL;
}

//if the memblock to insert into list of free memblocks is between two other free memblocks, use this function
static void insert_free_memblock_between(void* prevblock, void* nextblock, void* memblock){
	//Get all parts of the previous memory block
	int* prevblock_len = prevblock - sizeof(int);
	int** prevblock_prevptr = prevblock;
	int** prevblock_nextptr = prevblock + sizeof(int*);
	
	//Get all parts of the next memory block
	int* nextblock_len = nextblock - sizeof(int);
	int** nextblock_prevptr = nextblock;
	int** nextblock_nextptr = nextblock + sizeof(int*);
	
	//Get all parts of the memory block to insert
	int* memblock_len = memblock - sizeof(int);
	int** memblock_prevptr = memblock;
	int** memblock_nextptr = memblock + sizeof(int*);
	
	if (memblock_len - (int*)prevblock == *prevblock_len){
		*prevblock_len = *prevblock_len + *memblock_len + sizeof(int);
		return;
	}
	
	if (nextblock_len - (int*)memblock == *memblock_len){
		if (*nextblock_nextptr != NULL){
			int** nextnextblock_prevptr = (int**)*nextblock_nextptr;
			*nextnextblock_prevptr = memblock;
		}
		
		*prevblock_nextptr = memblock;
		*memblock_prevptr = prevblock;
		*memblock_nextptr = *nextblock_nextptr;
		
		*memblock_len = *memblock_len + *nextblock_len + sizeof(int);
		return;
	}
	
	*prevblock_nextptr = memblock;
	*memblock_prevptr = prevblock;
	*memblock_nextptr = nextblock;
	*nextblock_prevptr = memblock;
}

//if the memblock to insert into list of free memblocks is before the first free memblock, use this function
static void insert_free_memblock_beginning(void* memblock){
	//Get all parts of the memory block to insert
	int* memblock_len = memblock - sizeof(int);
	int** memblock_prevptr = memblock;
	int** memblock_nextptr = memblock + sizeof(int*);

	//get all parts of the first free memory block
	int* first_free_len = first_free - sizeof(int);
	int** first_free_prevptr = first_free;
	int** first_free_nextptr = first_free + sizeof(int*);
	
	if(first_free_len - (int*)memblock == *memblock_len){
		*memblock_len = *memblock_len + sizeof(int) + *first_free_len;
		*memblock_prevptr = NULL;
		*memblock_nextptr = first_free;
		first_free = memblock;
	}
	else {
		*memblock_prevptr = NULL;
		*memblock_nextptr = first_free;
		*first_free_prevptr = memblock;
		first_free = memblock;
	}
}

//if the memblock to insert into list of free memblocks is after the last free memblock, use this function
static void insert_free_memblock_end(void* endblock, void* memblock){
	//Get all parts of the memory block to insert
	int* memblock_len = memblock - sizeof(int);
	int** memblock_prevptr = memblock;
	int** memblock_nextptr = memblock + sizeof(int*);

	//get all parts of the last free memory block
	int* endblock_len = endblock - sizeof(int);
	int** endblock_prevptr = endblock;
	int** endblock_nextptr = endblock + sizeof(int*);
	
	if (memblock_len - (int*)endblock == *endblock_len){
		*endblock_len = *endblock_len + sizeof(int) + *memblock_len;
		*endblock_nextptr = memblock;
		*memblock_nextptr = NULL;
		*memblock_prevptr = endblock;
	}
	else {
		*memblock_nextptr = NULL;
		*memblock_prevptr = endblock;
		*endblock_nextptr = memblock;
	}
}

void my_free(void *ptr){
	if (first_free == NULL){
		int* tagbeg = ptr - 2 * sizeof(int);
		*tagbeg = false;
		int* blocklen = ptr - sizeof(int);
		int** pointerprev = ptr;
		*pointerprev = NULL;
		int** pointernext = ptr + sizeof(int*);
		*pointernext = NULL;
		int* tagend = ptr + *blocklen;
		*tagend = false;
		first_free = tagbeg;
		numfree++;
	}
	else {
		if(ptr < first_free){
			insert_free_memblock_beginning(ptr);
		}
		int* prev_free_block = first_free;
		//loop until we find the free block before the one we want to free
		while (*(int**)(prev_free_block + sizeof(int) + sizeof(int*)) < (int*)ptr){
			prev_free_block = *(int**)(prev_free_block + sizeof(int) + sizeof(int*));
		}
		void* next_free_block = *(int**)(prev_free_block + sizeof(int) + sizeof(int*));
		if(next_free_block == NULL){
			insert_free_memblock_end(prev_free_block, ptr);
		}
		else {
			insert_free_memblock_between(prev_free_block, next_free_block, ptr);
		}
	}
}

void my_mallopt(int policy){
	if (policy == FIRST_FIT || policy == BEST_FIT){
		alloc_policy = policy;
	}
}
void my_mallinfo();

int main(){
	printf("%p\n",my_malloc(300));
	printf("%p\n",my_malloc(300));
	printf("%p\n",my_malloc(300));
	return 0;
}
