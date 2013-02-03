#include <stdio.h>
#include <stdlib.h>
#include <alloca.h>

int global[5];
int global2[] = {0,1,2,3};
int main(int argc, char* argv){
    int i;
    int* j = (int*)malloc(sizeof(int));
    printf("Location: %p\n", &j);
    int* k = (int*)malloc(sizeof(int));
    printf("Location: %p\n", &k);
    int* l = (int*)alloca(sizeof(int));
    printf("Location: %p\n", &l);
    int* m = (int*)alloca(sizeof(int));
    printf("Location: %p\n", &m);
    printf("Location main: %p\n", main);
    printf("BSS starts at: %p\n", global);
    printf("Data starts at: %p\n", global2);
}