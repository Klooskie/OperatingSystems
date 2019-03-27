CC = gcc
CFLAGS = -Wall

all: client.c server.c
	$(CC) $(CFLAGS) client.c -o client -lrt
	$(CC) $(CFLAGS) server.c -o server -lrt

clean:
	rm client server
