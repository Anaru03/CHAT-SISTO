CC = gcc
CFLAGS = -Wall -pthread
LIBS = -lcjson

all: server

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

clean:
	rm -f server
