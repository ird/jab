CC = gcc
CFLAGS = -g -Wall -D_FILE_OFFSET_BITS=64
LIBS = -lcrypto

all: jab jabfs
	
jab: jab.o jab_common.o
	$(CC) $(CFLAGS) -o jab jab.o jab_common.o $(LIBS)

jab.o: jab.c
	$(CC) $(CFLAGS) -c -o jab.o -c jab.c

jabfs: jabfs.o jab_common.o
	$(CC) $(CFLAGS) -o jabfs jabfs.o jab_common.o -lfuse

jabfs.o: jabfs.c
	$(CC) $(CFLAGS) -c -o jabfs.o -c jabfs.c
	
jab_common.o: jab_common.c
	$(CC) $(CFLAGS) -c -o jab_common.o jab_common.c
	
clean:
	rm -rf *.o jab jabfs
