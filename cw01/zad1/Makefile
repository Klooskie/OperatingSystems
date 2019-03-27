CFLAGS = -Wall -O

all:
	make static
	make shared
	ls -l

static: tablelibdynamic.h tablelibdynamic.c tablelibstatic.h tablelibstatic.c
	gcc $(CFLAGS) -c tablelibstatic.c -o tablelibstatic.o
	ar rcs libtablelibstatic.a tablelibstatic.o

	gcc $(CFLAGS) -c tablelibdynamic.c -o tablelibdynamic.o
	ar rcs libtablelibdynamic.a tablelibdynamic.o

	make clean

shared: tablelibdynamic.h tablelibdynamic.c tablelibstatic.h tablelibstatic.c
	gcc $(CFLAGS) -c -fPIC tablelibstatic.c -o tablelibstatic.o
	gcc $(CFLAGS) -fPIC -shared -o libtablelibstatic.so tablelibstatic.o

	gcc $(CFLAGS) -c -fPIC tablelibdynamic.c -o tablelibdynamic.o
	gcc $(CFLAGS) -fPIC -shared -o libtablelibdynamic.so tablelibdynamic.o

	make clean

clean:
	rm *.o

clean2:
	rm *.a *.so
