#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

void usage_printer(struct rusage rusage1, struct rusage rusage2){
    printf("\x1B[32mWykorzystanie zasobow: \x1b[0m\n");

    struct timeval usr1, usr2, sys1, sys2;
    double time2, time1, usr, sys;

    usr1 = rusage1.ru_utime;
    usr2 = rusage2.ru_utime;
    sys1 = rusage1.ru_stime;
    sys2 = rusage2.ru_stime;

    time2 = (double) sys2.tv_sec + ((double) sys2.tv_usec) / 1000000;
    time1 = (double) sys1.tv_sec + ((double) sys1.tv_usec) / 1000000;
    usr = time2 - time1;

    printf("usr CPU time: %lf\n", usr);
    
    time2 = (double) usr2.tv_sec + ((double) usr2.tv_usec) / 1000000;
    time1 = (double) usr1.tv_sec + ((double) usr1.tv_usec) / 1000000;
    sys = time2 - time1;

    printf("sys CPU time: %lf\n", sys);

    printf("summed  time: %lf\n\n", usr + sys);

    fflush(stdout);
}


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
    if(argc != 4){
        printf("Bledne argumenty, uzycie:\n./main nazwaPlikuWsadowego ograniczenieCzasuProcesora ograniczeniePamieciWirtualnej\n");
        exit(1);
    }

    //otwarcie pliku
    FILE * file = fopen(argv[1], "r");
    if(file == NULL){
        perror("blad przy otwieraniu pliku wsadowego");
        exit(1);
    }

    //zapamietanie ograniczen w zmiennych
    struct rlimit timelimit, memlimit;
    timelimit.rlim_cur = atoi(argv[2]);
    timelimit.rlim_max = atoi(argv[2]);
    memlimit.rlim_cur = ((long long)atoi(argv[3])) * 1048576;
    memlimit.rlim_max = ((long long)atoi(argv[3])) * 1048576;

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

        //zapamietanie zasobow przed odpaleniem programu
        struct rusage rusage1, rusage2;
        if(getrusage(RUSAGE_CHILDREN, &rusage1) != 0){
            perror("blad przy zczytywaniu uzycia zasobow przed odpaleniem komendy");
            exit(2);
        }

        //stworzenie nowego procesu
        pid_t pid = fork();
        if(pid < 0){
            perror("blad przy tworzeniu nowego procesu");
            exit(1);
        }
        else if(pid == 0){

            //nalozenie ograniczen
            if(setrlimit(RLIMIT_CPU, &timelimit) != 0){
                perror("blad przy ustawianiu limitu czasu w trybie procesora");
                exit(2);
            }

            if(setrlimit(RLIMIT_AS, &memlimit) != 0){
                perror("blad przy ustawianiu limitu pamieci wirtualnej");
                exit(2);
            }

            //uruchomienie programu
            int exec_status = execvp(prog, args); 
            if(exec_status == -1){
                perror("\x1b[31m Blad przy probie wykonnania polecenia\x1b[0m");
                exit(2);
            }        
        }

        //oczekiwanie na zakonczenie procesu potomnego
        int wstatus;
        if(wait(&wstatus) == -1){
            perror("blad przy oczekiwaniu na proces potomny");
            exit(2);
        }

        //zapamietanie zasobow po zakonczeniu programu i wypisanie roznicy
        if(getrusage(RUSAGE_CHILDREN, &rusage2) != 0){
            perror("blad przy zczytywaiu uzycia zasobow po odpaleniu komendy");
            exit(2);
        }
        usage_printer(rusage1, rusage2);

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