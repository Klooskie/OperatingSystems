CC = gcc
CFLAGS = -Wall

all:
	$(CC) $(CFLAGS) server.c constants.c -o server -lpthread
	$(CC) $(CFLAGS) client.c constants.c -o client -lpthread
	
clean:
	rm -f server client
