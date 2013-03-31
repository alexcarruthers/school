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
	int* tagbeg;
	int* tagend;
	int* blocklen;
	void* data;
	void * memblock = sbrk(size + 3 * sizeof(int));
	tagbeg = memblock;
	*tagbeg = true;
	blocklen = memblock + sizeof(int);
	*blocklen = size;
	data = memblock + 2 * sizeof(int);
	tagend = memblock + size + 2 * sizeof(int);
	*tagend = true;
	return data;
}

/*static void* split_memblock(int size, void* memblock){
	int* memlength = memblock + sizeof(int);
	
	//checks to make sure that the remaining data can hold the beginning and
	//end tag as well as the pointers to the next and previous empty blocks
	if (*memlength < size + 3 * sizeof(int) + 2 * sizeof(void*)){
		return (void*)-1;
	}
	
	
}*/

extern char *my_malloc_error;
void *my_malloc(int size){
	//need to make sure when freeing the 
	if (size < 2 * sizeof(void*)){
		size = 2 * sizeof(void*);
	}
	
	if (!init){
		init = true;
		void* memblock = create_memblock(size);
		sbrk_start = memblock - 2 * sizeof(int);
		return memblock;
	}
	if (alloc_policy == FIRST_FIT){
		puts("test");
		void* memblock = sbrk_start;
		void* heap_end = sbrk(0);
		while (memblock < heap_end){
			int blocklength = *(int*)(memblock + sizeof(int));
			if (*(int*)memblock == true || blocklength < size){
				memblock += 3 * sizeof(int);
				continue;
			}
			if (blocklength > size){
				
			}
		}
	}
	
	return NULL;
}

static void insert_memblock(void* prevblock, void* nextblock, void* memblock){
	//Get all parts of the previous memory block
	int* prevblock_tagbeg = prevblock - 2 * sizeof(int);
	int* prevblock_len = prevblock - sizeof(int);
	int** prevblock_prevptr = prevblock;
	int** prevblock_nextptr = prevblock + sizeof(int*);
	int* prevblock_tagend = prevblock + *prevblock_len;
	
	//Get all parts of the next memory block
	int* nextblock_tagbeg = nextblock - 2 * sizeof(int);
	int* nextblock_len = nextblock - sizeof(int);
	int** nextblock_prevptr = nextblock;
	int** nextblock_nextptr = nextblock + sizeof(int*);
	int* nextblock_tagend = nextblock + *nextblock_len;
	
	//Get all parts of the memory block to insert
	int* memblock_tagbeg = memblock - 2 * sizeof(int);
	int* memblock_len = memblock - sizeof(int);
	int** memblock_prevptr = memblock;
	int** memblock_nextptr = memblock + sizeof(int*);
	int* memblock_tagend = memblock + *memblock_len;
	
	if (memblock_tagend - prevblock_tagbeg == sizeof(int)){
		*prevblock_len = *prevblock_len + *memblock_len + 3 * sizeof(int);
		*memblock_tagend = false;
		return;
	}
	
	if (nextblock_tagbeg - memblock_tagend == sizeof(int)){
		int** nextnextblock_prevptr = *nextblock_nextptr;
		
		*nextnextblock_prevptr = memblock;
		*prevblock_nextptr = memblock;
		*memblock_prevptr = prevblock;
		*memblock_nextptr = *nextblock_nextptr;
		
		*memblock_tagbeg = false;
		*memblock_len = *memblock_len + *nextblock_len + 3 * sizeof(int);
		return;
	}
	
	*memblock_tagbeg = false;
	*memblock_tagend = false;
	*prevblock_nextptr = memblock;
	*memblock_prevptr = prevblock;
	*memblock_nextptr = nextblock;
	*nextblock_prevptr = memblock;
}

//returns -1 if blocks are not contiguous, 0 on success
static int merge_memblocks(void* blk1, void* blk2){
	int* blk1_len = blk1 + sizeof(int);
	int* blk2_len = blk2 + sizeof(int);
	if (blk2 != blk1 + 3 * sizeof(int) + *blk1_len){
		return -1;
	}
	*blk1_len = *blk1_len + *blk2_len + 3 * sizeof(int);
	return 0;
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
		int* prev_free_block = first_free;
		//loop until we find the free block before the one we want to free
		while (*(prev_free_block + 3 * sizeof(int)) < (int*)ptr){
			prev_free_block = *(prev_free_block + 3 * sizeof(int));
		}
		void* next_free_block = *(prev_free_block + 3 * sizeof(int));
		
		int* tagbeg = ptr - 2 * sizeof(int);
		*tagbeg = false;
		
	}
}
void my_mallopt(int policy){
	if (policy == FIRST_FIT || policy == BEST_FIT){
		alloc_policy = policy;
	}
}
void my_mallinfo();

int main(){
	printf("%d\n",my_malloc(300));
	printf("%d\n",my_malloc(300));
	printf("%d\n",my_malloc(300));
	return 0;
}
