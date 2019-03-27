CFLAGS = -Wall -O

all:
	gcc $(CFLAGS) -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS) -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o
	gcc $(CFLAGS) -I ../zad1 -c main.c -o main.o 
	gcc $(CFLAGS) main.o tablelibstatic.o tablelibdynamic.o -o main	

	make clean
	make test

clean:
	rm *.o

test:
	./main 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> raport2.txt
	./main 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> raport2.txt

clean2:
	rm main raport2.txt