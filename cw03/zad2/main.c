#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

char * getname(char * prog){

    int i = 0, shift = 0;
    while(prog[i] != '\0'){
        if(prog[i] == '/')
            shift = i + 1;
        i++;
    }

    return prog + shift;
}

int main(int argc, char ** argv){
    
    //obsluga niepoprawnej ilosci arguemntow
    if(argc != 2){
        printf("Bledne argumenty, uzycie:\n./main nazwaPlikuWsadowego\n");
        exit(1);
    }

    //otwarcie pliku
    FILE * file = fopen(argv[1], "r");
    if(file == NULL){
        perror("blad przy otwieraniu pliku wsadowego");
        exit(1);
    }

    char * buffer = NULL;
    size_t buf_size;

    //odczytanie pierwszej linii
    int size = getline(&buffer, &buf_size, file);

    while(size != -1){

        //odciecie bialych znakow na koncu linii
        while(buffer[size-1] == ' ' || buffer[size-1] == '\n'){
            buffer[size-1] = '\0';
            size--;
        }

        //wypisanie instrukcji i zapisanie kopii
        char * bufbackup = malloc(size * sizeof(char));
        strcpy(bufbackup, buffer);
        printf("\x1B[32mInstrukcja: \"%s\"\x1b[0m\n", buffer);

        //stworzenie tablicy kolejnych argumentow
        char ** args = malloc(101 * sizeof(char *));
        char * prog = strtok(buffer, " \n\t");
        args[0] = getname(prog);

        //odczytanie kolejnych argumentow
        int i = 0;
        while(args[i] != NULL){
            i++;

            if(i == 101){
                printf("przekroczono maksymalna liczbe argumentow (nazwa_programu + 99 argumentow)");
                exit(1);
            }
               
            args[i] = strtok(NULL, " \n\t");
        }
        
        //stworzenie nowego procesu
        pid_t pid = fork();
        if(pid < 0){
            perror("blad przy tworzeniu nowego procesu");
            exit(1);
        }
        else if(pid == 0){

            //uruchomienie programu
            int exec_status = execvp(prog, args); 
            if(exec_status == -1){
                perror("\x1b[31m Blad przy probie wykonnania polecenia\x1b[0m");
                exit(2);
            }
        
        }

        //oczekiwanie na zakonczenie procesu potomnego
        int wstatus;
        wait(&wstatus);

        //obsluga zakonczenia sygnalem
        if(WIFSIGNALED(wstatus)){
            printf("\x1b[31m Proces z poleceniem \"%s\" zakonczyl sie ze statusem innym niz 0, zostal zabity sygnalem %d\n", bufbackup, WTERMSIG(wstatus));
            exit(2);
        }

        //obsluga zakonczenia normalnego
        if(WIFEXITED(wstatus) && WEXITSTATUS(wstatus) != 0){
            printf("\x1b[31m Proces z poleceniem \"%s\" zakonczyl sie ze statusem innym niz 0 (%d) \x1b[0m \n", bufbackup, WEXITSTATUS(wstatus));
            exit(2);
        }
        
        //wczytanie kolejnej linii z pliku wsadowego
        printf("\n");
        size = getline(&buffer, &buf_size, file);
    }

    //zwolnienie bufora na linie i zamkniecie pliku
    free(buffer);
    int close = fclose(file);
    if(close != 0){
        perror("blad przy zamykaniu pliku wsadowego");
        exit(1);
    }
}