CC = gcc
CFLAGS = -Wall -pthread -I/usr/local/include/cjson
LIBS = -L/usr/local/lib -lcjson

SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): server.c
	$(CC) $(CFLAGS) server.c -o $(SERVER) $(LIBS)

$(CLIENT): client.c
	$(CC) $(CFLAGS) client.c -o $(CLIENT) $(LIBS)

clean:
	rm -f $(SERVER) $(CLIENT)
