CC = gcc
CFLAGS = -DHAVE_PTHREAD_RWLOCK=1 -lslack -lm
OBJECTS = my-test.o mythread.o

main.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o mythread-test $(CFLAGS)
	rm *.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<
