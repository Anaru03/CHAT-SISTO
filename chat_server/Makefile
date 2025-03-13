CC = gcc
CFLAGS = -Wall -pthread -I"C:/Program Files (x86)/cJSON/include"
LIBS = -L"C:/Program Files (x86)/cJSON/lib" -lcjson

all: server

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

clean:
	rm -f server
