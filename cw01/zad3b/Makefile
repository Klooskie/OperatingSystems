OUTPUT = raport3b.txt
CFLAGS1 = -Wall -O0
CFLAGS2 = -Wall -Os
CFLAGS3 = -Wall -O3

all:
	make static1
	make static2
	make static3
	make shared1
	make shared2
	make shared3
	make dynamic1
	make dynamic2
	make dynamic3

static1:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek statycznych" >> $(OUTPUT)

	gcc $(CFLAGS1) -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS1) -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o
	
	ar rcs libtablelibstatic.a tablelibstatic.o
	ar rcs libtablelibdynamic.a tablelibdynamic.o 

	gcc $(CFLAGS1) -I ../zad1 -c main.c -o main.o
	gcc $(CFLAGS1) main.o -ltablelibstatic -ltablelibdynamic -L./ -o main_static

	./main_static 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_static 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

static2:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek statycznych" >> $(OUTPUT)

	gcc $(CFLAGS2) -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS2) -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o
	
	ar rcs libtablelibstatic.a tablelibstatic.o
	ar rcs libtablelibdynamic.a tablelibdynamic.o 

	gcc $(CFLAGS2) -I ../zad1 -c main.c -o main.o
	gcc $(CFLAGS2) main.o -ltablelibstatic -ltablelibdynamic -L./ -o main_static

	./main_static 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_static 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

static3:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek statycznych" >> $(OUTPUT)

	gcc $(CFLAGS3) -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS3) -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o
	
	ar rcs libtablelibstatic.a tablelibstatic.o
	ar rcs libtablelibdynamic.a tablelibdynamic.o 

	gcc $(CFLAGS3) -I ../zad1 -c main.c -o main.o
	gcc $(CFLAGS3) main.o -ltablelibstatic -ltablelibdynamic -L./ -o main_static

	./main_static 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_static 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

shared1:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek dzielonych" >> $(OUTPUT)

	gcc $(CFLAGS1) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS1) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS1) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS1) -shared -o libtablelibdynamic.so tablelibdynamic.o

	gcc $(CFLAGS1) -I ../zad1 main.c -o main_shared -L./ -ltablelibstatic -ltablelibdynamic -Wl,-rpath=./
	
	./main_shared 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_shared 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

shared2:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek dzielonych" >> $(OUTPUT)

	gcc $(CFLAGS2) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS2) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS2) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS2) -shared -o libtablelibdynamic.so tablelibdynamic.o

	gcc $(CFLAGS2) -I ../zad1 main.c -o main_shared -L./ -ltablelibstatic -ltablelibdynamic -Wl,-rpath=./
	
	./main_shared 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_shared 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

shared3:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek dzielonych" >> $(OUTPUT)

	gcc $(CFLAGS3) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS3) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS3) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS3) -shared -o libtablelibdynamic.so tablelibdynamic.o

	gcc $(CFLAGS3) -I ../zad1 main.c -o main_shared -L./ -ltablelibstatic -ltablelibdynamic -Wl,-rpath=./
	
	./main_shared 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_shared 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

dynamic1:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek ladowanych dynamicznie" >> $(OUTPUT)

	#najpierw tworze biblioteke .so
	gcc $(CFLAGS1) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS1) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS1) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS1) -shared -o libtablelibdynamic.so tablelibdynamic.o
	
	#potem normalnie kompiluje program
	gcc $(CFLAGS1) -I ../zad1 main_dyn.c -o main_dynamic -ldl

	./main_dynamic 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_dynamic 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

dynamic2:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek ladowanych dynamicznie" >> $(OUTPUT)

	#najpierw tworze biblioteke .so
	gcc $(CFLAGS2) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS2) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS2) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS2) -shared -o libtablelibdynamic.so tablelibdynamic.o
	
	#potem normalnie kompiluje program
	gcc $(CFLAGS2) -I ../zad1 main_dyn.c -o main_dynamic -ldl

	./main_dynamic 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_dynamic 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

dynamic3:
	echo "\n\nKompilacja i uruchomienie z wykorzystaniem bibliotek ladowanych dynamicznie" >> $(OUTPUT)

	#najpierw tworze biblioteke .so
	gcc $(CFLAGS3) -fPIC -c ../zad1/tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS3) -fPIC -c ../zad1/tablelibdynamic.c -o tablelibdynamic.o

	gcc $(CFLAGS3) -shared -o libtablelibstatic.so tablelibstatic.o
	gcc $(CFLAGS3) -shared -o libtablelibdynamic.so tablelibdynamic.o
	
	#potem normalnie kompiluje program
	gcc $(CFLAGS3) -I ../zad1 main_dyn.c -o main_dynamic -ldl

	./main_dynamic 1000 1000 s create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)
	./main_dynamic 1000 1000 d create 500 500 add 10000 search 1 remove 200 add 200 remove_and_add 200 >> $(OUTPUT)

	make clean

clean:
	rm -f *.o *.a *.so

clean2:
	rm -f main_static main_shared main_dynamic $(OUTPUT)