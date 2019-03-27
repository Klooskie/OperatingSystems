CC = gcc
CFLAGS = -Wall

all: client.c server.c
	$(CC) $(CFLAGS) client.c -o client
	$(CC) $(CFLAGS) server.c -o server

clean:
	rm client server
