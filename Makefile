CC = gcc
CFLAGS = -g -Wall
LIBS = -lcrypto -lsqlite3

all: backup
	
backup: backup.o configuration.o database.o hash.o path.o source.o target.o transfer.o
	$(CC) $(CFLAGS) -o backup backup.o configuration.o database.o hash.o path.o source.o target.o transfer.o $(LIBS)

backup.o: backup.c
	$(CC) $(CFLAGS) -o backup.o -c backup.c
	
configuration.o: configuration.h configuration.c
	$(CC) $(CFLAGS) -o configuration.o -c configuration.c
	
database.o: database.h database.c
	$(CC) $(CFLAGS) -o database.o -c database.c

hash.o: hash.h hash.c
	$(CC) $(CFLAGS) -o hash.o -c hash.c
	
path.o: path.h path.c
	$(CC) $(CFLAGS) -o path.o -c path.c

source.o: source.h source.c
	$(CC) $(CFLAGS) -o source.o -c source.c

target.o: target.h target.c
	$(CC) $(CFLAGS) -o target.o -c target.c

transfer.o: transfer.h transfer.c
	$(CC) $(CFLAGS) -o transfer.o -c transfer.c

clean:
	rm -rf *.o backup
