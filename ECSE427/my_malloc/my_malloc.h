#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>


extern char *my_malloc_error;
void *my_malloc(int size);

void my_free(void *ptr);

void my_mallopt(int policy);

void my_mallinfo();
#endif
