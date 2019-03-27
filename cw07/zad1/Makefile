CC = gcc
CFLAGS = -Wall

all: barber.c clients.c shop_lib.c
	$(CC) $(CFLAGS) barber.c shop_lib.c -o barber
	$(CC) $(CFLAGS) clients.c shop_lib.c -o clients

clean:
	rm barber clients
