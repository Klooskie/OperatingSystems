#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int running = 0;
int pid;
int parent_signals_got = 0;
int parent_signals_sent = 0;
int child_signals_got = 0;
sigset_t empty_mask;
sigset_t full_mask;
sigset_t child_mask;

void SIGUSR1_parent_support(int signum){
    parent_signals_got++;
}

void SIGINT_parent_support(int signum){
    //zabicie procesu potomnego jesli uruchomiony
    if(running == 1)
        kill(pid, SIGUSR2);
    
    //wypisanie licznikow sygnalow wyslanych i otrzymanych przez rodzica
    printf("liczba sygnalow wysylanych przez rodzica do potomka: %d\nliczba sygnalow odebranych przez rodzica do potomka: %d\n", parent_signals_sent, parent_signals_got);

    //zakonczenie programu
    exit(0);
}

void SIGUSR1_child_support(int signum){
    child_signals_got++;
    kill(getppid(), SIGUSR1);
}

void SIGUSR2_child_support(int signum){
    child_signals_got++;
    printf("liczba sygnalow odebranych przez potomka od rodzica: %d\n", child_signals_got);
    exit(0);
}

void child_RT(){
    //ustawienie handlerow dla dziecka
    struct sigaction act;
    act.sa_handler = SIGUSR1_child_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGRTMIN+1, &act, NULL);

    act.sa_handler = SIGUSR2_child_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGRTMIN+2, &act, NULL);

    //zczytywanie odpowiednich 2 sygnalow, suspend pozwala na zostawienie maski kryjacej wszytkie sygnaly, zeby proces nie dostal sygnalu zanim ustawi handlery
    while(1){
        sigsuspend(&child_mask);
    }
}

void child(){
    //ustawienie handlerow dla dziecka
    struct sigaction act;
    act.sa_handler = SIGUSR1_child_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = SIGUSR2_child_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR2, &act, NULL);

    //zczytywanie odpowiednich 2 sygnalow, suspend pozwala na zostawienie maski kryjacej wszytkie sygnaly, zeby proces nie dostal sygnalu zanim ustawi handlery
    while(1){
        sigsuspend(&child_mask);
    }
}

void createChild(int i){
    //ustawienie maski blokujacej wszystkie sygnaly
    if(sigprocmask(SIG_SETMASK, &full_mask, &empty_mask) != 0){
        perror("blad przy ustawianiu maski sygnalow");
        exit(1);
    }

    //utworzenie nowego procesu
    pid = fork();
    if(pid < 0){
        perror("blad przy tworzeniu procesu potomnego");
        exit(1);
    }
    else if(pid == 0){
        //uruchamianie odpowiedniej funkcji w procesie potomnym
        if(i <= 2)
            child();
        else
            child_RT();
    }
    else{
        //ustawianie zmiennej informujacej o dzialaniu procesu potomnego
        running = 1;
    }
}

int main(int argc, char ** argv){
    //ustawienie handlerow dla rodzica
    struct sigaction act;
    act.sa_handler = SIGUSR1_parent_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = SIGINT_parent_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    //stworzenie pelnej maski
    if(sigfillset(&full_mask) != 0){
        perror("blad przy tworzeniu pelnej maski");
        exit(1);
    }

    if(argc != 3){
        printf("Zla ilosc argumentow, uzycie:\n./main liczbaSygnalowDoWyslania typ\n");
        exit(1);
    }

    int L = atoi(argv[1]);
    int Type = atoi(argv[2]);

    if(Type == 1){
        //stworzenie nowej maski sygnalow dla dziecka
        if(sigfillset(&child_mask) != 0){
            perror("blad przy tworzeniu maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGUSR1) != 0){
            perror("blad przy usuwaniu SIGUSR1 z maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGUSR2) != 0){
            perror("blad przy usuwaniu SIGUSR2 z maski sygnalow");
            exit(1);
        }

        //stworzenie dziecka z ustawieniem pelnej maski
        createChild(1);
        
        //ustawienie poczatkowej maski dla procesu macierzystego
        if(sigprocmask(SIG_SETMASK, &empty_mask, NULL) != 0){
            perror("blad przy ustawianiu maski dla rodzica");
            exit(1);
        }

        //wyslanie sygnalu SIGUSR1 do potomka L razy, a potem SIGUSR2
        while(L--){
            kill(pid, SIGUSR1);
            parent_signals_sent++;
        }
        kill(pid, SIGUSR2);

        //wypisanie licznikow sygnalow wyslanych i otrzymanych przez rodzica
        printf("liczba sygnalow wysylanych przez rodzica do potomka: %d\nliczba sygnalow odebranych przez rodzica do potomka: %d\n", parent_signals_sent, parent_signals_got);
    }
    else if(Type == 2){
        //stworzenie nowej maski sygnalow dla dziecka
        if(sigfillset(&child_mask) != 0){
            perror("blad przy tworzeniu maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGUSR1) != 0){
            perror("blad przy usuwaniu SIGUSR1 z maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGUSR2) != 0){
            perror("blad przy usuwaniu SIGUSR2 z maski sygnalow");
            exit(1);
        }

        //stworzenie dziecka z ustawieniem pelnej maski
        createChild(2);

        //wyslanie L sygnalow SIGUSR1 i SIGUSR2
        while(L--){
            kill(pid, SIGUSR1);
            parent_signals_sent++;
            //oczekiwanie na dowolny sygnal, suspend pozwala uzyc pelnej maski, aby nie przegapic zadnego sygnalu od potomka
            sigsuspend(&empty_mask);
        }
        kill(pid, SIGUSR2);

        //wypisanie licznikow sygnalow wyslanych i otrzymanych przez rodzica
        printf("liczba sygnalow wysylanych przez rodzica do potomka: %d\nliczba sygnalow odebranych przez rodzica do potomka: %d\n", parent_signals_sent, parent_signals_got);
    }
    else if(Type == 3){
        //stworzenie nowej maski sygnalow dla dziecka
        if(sigfillset(&child_mask) != 0){
            perror("blad przy tworzeniu maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGRTMIN+1) != 0){
            perror("blad przy usuwaniu SIGRTMIN+1 z maski sygnalow");
            exit(1);
        }
        if(sigdelset(&child_mask, SIGRTMIN+2) != 0){
            perror("blad przy usuwaniu SIGRTMIN+2 z maski sygnalow");
            exit(1);
        }

        //stworzenie dziecka z ustawieniem pelnej maski
        createChild(3);

        //ustawienie poczatkowej maski dla procesu macierzystego
        if(sigprocmask(SIG_SETMASK, &empty_mask, NULL) != 0){
            perror("blad przy ustawianiu maski dla rodzica");
            exit(1);
        }

        //wyslanie sygnalu SIGRTMIN+1 do potomka L razy, a potem SIGRTMIN+2
        while(L--){
            kill(pid, SIGRTMIN+1);
            parent_signals_sent++;
        }
        kill(pid, SIGRTMIN+2);

        //wypisanie licznikow sygnalow wyslanych i otrzymanych przez rodzica
        printf("liczba sygnalow wysylanych przez rodzica do potomka: %d\nliczba sygnalow odebranych przez rodzica do potomka: %d\n", parent_signals_sent, parent_signals_got);    
    }
    else{
        printf("Zla wartosc argumentu 'typ', oczekwiana wartosc 1, 2 lub 3\n");
        exit(1);
    }

    exit(0);
}