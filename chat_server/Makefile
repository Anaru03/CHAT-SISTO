CC = gcc
CFLAGS = -Wall -pthread -I"C:/Program Files (x86)/cJSON/include"
LIBS = -L"C:/Program Files (x86)/cJSON/lib" -lcjson

SERVER = server
CLIENT = client

all: $(SERVER) $(CLIENT)

$(SERVER): server.c
	$(CC) $(CFLAGS) server.c -o $(SERVER) $(LIBS)

$(CLIENT): client.c
	$(CC) $(CFLAGS) client.c -o $(CLIENT) $(LIBS)

clean:
	rm -f $(SERVER) $(CLIENT)
