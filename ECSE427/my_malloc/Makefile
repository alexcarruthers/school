CC = gcc
OBJECTS = my_malloc.o my_malloc_test.o

main.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o my_malloc-test
	rm *.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<
