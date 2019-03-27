#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char ** argv){

    //sprawdzenie poprawnosci argumentow programu
    if(argc != 3){
        printf("zla liczba argumentow, uzycie:\n./slave sciezka_do_potoku liczba_pisan_do_fifo\n");
        exit(1);
    }    

    char * path = argv[1];
    int N = atoi(argv[2]);

    //otwarcie fifo
    FILE * fifo = fopen(path, "a");
    if(fifo == NULL){
        perror("blad przy otwieraniu fifo");
        exit(1);
    }

    //wypisanie na stdout informacji o rozpoczeciu procesu
    printf("Proces slave o pid: %d, z argumentem N = %d\n", getpid(), N);

    for(int i = 0; i < N; i++){

        //otwarcie programu date z dostepem do jego wyjscie przez potok nienazwany
        FILE * unnamed = popen("date", "r");
        if(unnamed == NULL){
            perror("blad przy otwieraniu potoku nienazwanego w celu uzyskania daty");
            exit(1);
        }

        //odczytanie daty z potoku
        char * date = malloc(32);
        fread(date, 1, 32, unnamed);

        //zamkniecie potoku do odczytu daty
        if(pclose(unnamed) < 0){
            perror("blad przy zamykaniu potoku nienazwanego");
            exit(1);
        }

        //stworzenie linii do wpisania do fifo
        char * line = malloc(64);
        sprintf(line, "Pid %d; Data ", getpid());
        strcat(line, date);
        strcat(line, "\n");
        
        //dopisanie linii do potoku
        if(fwrite(line, 1, strlen(line), fifo) < strlen(line)){
            perror("blad przy zapisie do fifo");
            exit(1);
        }

        fflush(fifo);

        //odczekanie 1-3 sekund
        sleep(rand() % 3 + 1);
    }

    if(fclose(fifo) != 0){
        perror("blad przy zamykaniu potoku nazwanego");
        exit(1);
    }
}