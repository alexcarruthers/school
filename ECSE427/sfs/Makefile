CC = gcc -Wall
CFLAGS = -DHAVE_PTHREAD_RWLOCK=1 -lslack -lm
OBJECTS = sfs_api.o sfs_test.o disk_emu.o

main.exe : $(OBJECTS)
	$(CC) $(OBJECTS) -o sfs_test $(CFLAGS)
	rm *.o

%.o : %.c
	$(CC) $(CFLAGS) -c $<
