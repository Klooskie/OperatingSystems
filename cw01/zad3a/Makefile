OUTPUT = raport3a.txt
CFLAGS = -Wall -O

all:
	make static
	make shared
	make dynamic

static:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek statycznych" >> $(OUTPUT)

	gcc $(CFLAGS) -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS) -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o
	
	ar rcs libtablelibstatic.a tablelibstatic.o
	ar rcs libtablelibdynamic.a tablelibdynamic.o 

	gcc $(CFLAGS) -I ../zad1 -c main.c -o main.o
	gcc $(CFLAGS) main.o -ltablelibstatic -ltablelibdynamic -L./ -o main_static

	./main_static 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_static 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

shared:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek dzielonych" >> $(OUTPUT)

	gcc $(CFLAGS) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS) -shared -o libtablelibdynamic.so tablelibdynamic.o

	gcc $(CFLAGS) -I ../zad1 main.c -o main_shared -L./ -ltablelibstatic -ltablelibdynamic -Wl,-rpath=./
	
	./main_shared 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_shared 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

dynamic:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek ladowanych dynamicznie" >> $(OUTPUT)

	#najpierw tworze biblioteke .so
	gcc $(CFLAGS) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS) -shared -o libtablelibdynamic.so tablelibdynamic.o
	
	#potem normalnie kompiluje program
	gcc $(CFLAGS) -I ../zad1 main_dyn.c -o main_dynamic -ldl

	./main_dynamic 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_dynamic 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

clean:
	rm -f *.o *.a *.so

clean2:
	rm -f main_static main_shared main_dynamic $(OUTPUT)