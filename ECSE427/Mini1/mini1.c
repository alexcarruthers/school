#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>
#include <sys/resource.h>

extern unsigned long __executable_start;
extern unsigned long __etext;
extern unsigned long edata;
extern unsigned long end;
int main(int argc, char* argv){
	struct rlimit stack_limit;
	struct rlimit data_limit;
	getrlimit(RLIMIT_STACK, &stack_limit);
	getrlimit(RLIMIT_DATA, &data_limit);
	printf("Text segment           starts at: 0x%x ends at: 0x%x\n",&__executable_start, &__etext);
	printf("Data segment           starts at: 0x%x ends at: 0x%x\n",&__etext, &edata);
	printf("BSS segment            starts at: 0x%x ends at: 0x%x\n",&edata, &end);
	printf("Heap                   starts at: 0x%x ends at: %p\n", &end, sbrk(0));
	printf("Memory mapping segment starts at: %p ends at: %p\n", sbrk(0), stack_limit.rlim_cur);
	printf("Stack                  starts at: %p ends at: %p\n", stack_limit.rlim_cur, argv);
}
