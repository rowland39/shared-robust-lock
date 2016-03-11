UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Linux)
    CFLAGS=-pthread
    LDFLAGS=-pthread
    CC=gcc
endif

ifeq ($(UNAME_S),FreeBSD)
    CFLAGS=-pthread
    LDFLAGS=-pthread
    CC=cc
endif

ifeq ($(UNAME_S),Darwin)
    CFLAGS=
    LDFLAGS=-lpthread
    CC=cc
endif

.PHONY: all
all: server-mmap client-mmap server-shm client-shm

server-mmap: server-mmap.o
	$(CC) -o $@ $^ $(LDFLAGS)

server-mmap.o: server-mmap.c
	$(CC) -c $< $(CFLAGS)

client-mmap: client-mmap.o
	$(CC) -o $@ $^ $(LDFLAGS)

client-mmap.o: client-mmap.c
	$(CC) -c $< $(CFLAGS)

server-shm: server-shm.o
	$(CC) -o $@ $^ $(LDFLAGS)

server-shm.o: server-shm.c
	$(CC) -c $< $(CFLAGS)

client-shm: client-shm.o
	$(CC) -o $@ $^ $(LDFLAGS)

client-shm.o: client-shm.c
	$(CC) -c $< $(CFLAGS)

clean:
	rm -f *.o server-mmap client-mmap server-shm client-shm
