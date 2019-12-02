
# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
PROGS = cli
SOURCES = server.c fs.c sync.c 
SOURCES+= lib/bst.c lib/inodes.c 
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_RWLOCK)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS =tecnicofs clients

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


lib/bst-rwlock.o: lib/bst.c lib/bst.h

lib/inodes-rwlock.o: lib/inodes.c lib/inodes.h

fs-rwlock.o: fs.c fs.h lib/bst.h lib/inodes.h sync.h tecnicofs-api-constants.h

sync-rwlock.o: sync.c sync.h constants.h 

server-rwlock.o: server.c fs.h lib/bst.h constants.h lib/timer.h sync.h tecnicofs-api-constants.h

tecnicofs: lib/inodes-rwlock.o lib/bst-rwlock.o fs-rwlock.o sync-rwlock.o server-rwlock.o

api-tests/client-api-test-create.o: api-tests/client-api-test-create.c cliente/tecnicofs-client-api.h tecnicofs-api-constants.h
	$(CC) $(CFLAGS) -o api-tests/client-api-test-create.o -c api-tests/client-api-test-create.c

cliente/tecnicofs-api-client.o: cliente/tecnicofs-api-client.c cliente/tecnicofs-client-api.h
	$(CC) $(CFLAGS) -o cliente/tecnicofs-api-client.o -c cliente/tecnicofs-api-client.c

clients: api-tests/client-api-test-create.o cliente/tecnicofs-api-client.o
	$(CC) $(CFLAGS) -o api-tests/client-api-test-create api-tests/client-api-test-create.o cliente/tecnicofs-api-client.o



%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
	
