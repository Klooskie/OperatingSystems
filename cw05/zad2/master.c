#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char ** argv){

    //sprawdzenie poprawnosci argumentow programu
    if(argc != 2){
        printf("zla liczba argumentow, uzycie:\n./master sciezka_do_potoku\n");
        exit(1);
    }

    const char * path = argv[1];

    //utworzenie potoku nazwanego
    if(mkfifo(path, S_IWUSR | S_IRUSR) < 0){
        perror("blad przy tworzeniu potoku nazwanego");
        exit(1);
    }

    //otwarcie fifo
    FILE * fifo = fopen(path, "r");
    if(fifo == NULL){
        perror("blad przy otwieraniu fifo");
        exit(1);
    }

    char * buf;

    //odczytywanie linii
    size_t size;
    while(getline(&buf, &size, fifo) > 0){
        printf("%s", buf);
    }

    //zamkniecie fifo
    if(fclose(fifo) != 0){
        perror("blad przy zamykaniu pliku");
        exit(1);
    }

    if(remove(path) < 0){
        perror("blad przy usuwaniu potoku nazwanego");
        exit(1);
    }

}