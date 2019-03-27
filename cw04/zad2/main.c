#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

//zmienne odpowiedzialne za wyswietlanie poszczegolnych informacji !!!
int print_process_creation = 1;
int print_getting_SIGUSR1 = 1;
int print_sending_SIGCONT = 1;
int print_got_RT_signals = 1;
int print_process_death = 1;

//pozostale zmienne globalne
int N;
int K;
int number_of_waiting_children = 0;
int children_finished = 0;

pid_t * children;
pid_t * waiting_children;
int * working;

sigset_t empty_mask;
sigset_t full_mask;
sigset_t child_mask;


void SIGUSR1_support(int signo, siginfo_t * siginfo, void * ucontex){
    if(number_of_waiting_children < K){
        //zanim otrzymamy K prosb dodajemy pid do tablicy oczekujacych
        waiting_children[number_of_waiting_children] = siginfo -> si_pid;
        
        //wypisanie informacji o otrzymaniu SIGUSR1
        if(print_getting_SIGUSR1 == 1)
            printf("Otrzymano SIGUSR1 od pid -> %d\n", siginfo -> si_pid);
        
        number_of_waiting_children++;
    }
    else if(number_of_waiting_children == K){
        //w przypadku otrzymania K-tej prosby wysylamy SIGCONT do wszystkich oczekujacych

        //wypisanie informacji o otrzymaniu SIGUSR1
        if(print_getting_SIGUSR1 == 1)
            printf("Otrzymano SIGUSR1 od pid -> %d byla to K-ta prosba\n", siginfo -> si_pid);

        //wyslanie SIGCONT do kazdego czekajacego procesu
        for(int i = 0; i < number_of_waiting_children; i++){
            //wypisanie informacji o wyslaniu SIGCONT
            if(print_sending_SIGCONT == 1)
                printf("Wyslano SIGCONT do procesu potomnego o pid %d\n", waiting_children[i]);
            
            kill(waiting_children[i], SIGCONT);
        }
        if(print_sending_SIGCONT == 1) 
            printf("Wyslano SIGCONT do procesu potomnego o pid %d\n", siginfo -> si_pid);
        
        kill(siginfo -> si_pid, SIGCONT);
        
        number_of_waiting_children++;   
    }
    else{
        //wypisanie informacji o otrzymaniu SIGUSR1
        if(print_getting_SIGUSR1 == 1)
            printf("Otrzymano SIGUSR1 od pid -> %d\n", siginfo -> si_pid);
        
        //jak otrzymalismy wiecej niz K to wysylamy SIGCONT od razu
        if(print_sending_SIGCONT == 1) 
            printf("Wyslano SIGCONT do procesu potomnego o pid %d\n", siginfo -> si_pid);
        kill(siginfo -> si_pid, SIGCONT);
    }    
}

void SIGCONT_support(int signo){
    //wylosowanie i wyslanie sygnalu czasu rzeczywistego do rodzica
    int rt_sig_number = rand() % 32;
    kill(getppid(), SIGRTMIN + rt_sig_number);
}

void SIGINT_support(int signo){
    //zabicie wszyskich dzialajacych potomkow
    for(int i = 0; i < N; i++){
        if(working[i] == 1){
            printf("zabicie potomka o pid %d\n", children[i]);
            
            kill(children[i], SIGKILL);
        }
    }

    //zakonczenie main'a
    exit(0);
}

void SIGRT_support(int signo, siginfo_t * siginfo, void * ucontex){
    if(print_got_RT_signals == 1)
        printf("Otrzymano sygnal RT %d od %d\n", signo - SIGRTMIN , siginfo -> si_pid);
}

void SIGCHLD_support(int signo){
    //odczytanie pid zakonczonego procesu
    int status;
    pid_t ended_child_pid = waitpid(-1, &status, WNOHANG);

    //odczytanie pid pozostalych zakonczonych procesow
    while(ended_child_pid > 0){

        //zwiekszenie liczby zakonczonych procesow
        children_finished++;

        //wypisanie informacji o zakonczeniu procesu i jego zwroconej wartosci
        if(print_process_death == 1)
            printf("Dziecko zakonczylo sie: pid %d, zwrocona wartosc: %d\n", ended_child_pid, WEXITSTATUS(status));

        //aktualizacja stanu dziecka na zakonczony
        int i;
        for(int j = 0; j < N; j++)
            if(children[j] == ended_child_pid)
                i = j;  
        working[i] = 0;

        ended_child_pid = waitpid(-1, &status, WNOHANG);
    }
}

void child(){
    //ustawienie handlerow
    struct sigaction act;
    //SIGCONT
    act.sa_handler = SIGCONT_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGCONT, &act, NULL) != 0){
        perror("blad przu ustawianiu handlera SIGCONT dla dziecka");
        exit(1);
    }

    //przespanie 0-10 sekund
    int time_to_sleep = rand() % 11;
    sleep(time_to_sleep);

    //wyslanie sygnalu do procesu macierzystego
    kill(getppid(), SIGUSR1);

    //oczekiwanie na sygnal SIGCONT
    sigsuspend(&child_mask);

    exit(time_to_sleep);
}

void create_child(int i){
    //ustawienie maski blokujacej wszystkie sygnaly
    if(sigprocmask(SIG_SETMASK, &full_mask, &empty_mask) != 0){
        perror("blad przy ustawianiu maski sygnalow");
        exit(1);
    }

    //stworzenie nowego procesu
    pid_t pid = fork();
    if(pid < 0){
        perror("blad przy tworzeniu procesu potomnego");
        exit(1);
    }
    else if(pid == 0){
        child();
    }
    else{
        //powrot do starej maski
        if(sigprocmask(SIG_SETMASK, &empty_mask, NULL) != 0){
            perror("blad przy ustawianiu maski sygnalow");
            exit(1);
        }

        //dodanie dziecka do listy
        children[i] = pid;
        working[i] = 1;

        //wypisanie kommunikatu o stworzeniu procesu potomnego
        if(print_process_creation == 1)
            printf("Utworzono proces potomny o pid: %d\n", pid);
    }
}

int main(int argc, char ** argv){
    //ustawienie handlerow
    struct sigaction act;
    //SIGINT
    act.sa_handler = SIGINT_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction(SIGINT, &act, NULL) != 0){
        perror("blad przu ustawianiu handlera SIGINT dla rodzica");
        exit(1);
    }
    //SIGRTMIN - SIGRTMAX
    act.sa_sigaction = SIGRT_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    for(int i = SIGRTMIN; i <= SIGRTMAX; i++)
        if(sigaction(i, &act, NULL) != 0){
            perror("blad przu ustawianiu handlera sygnalu czasu rzeczywistego dla rodzica");
            exit(1);
        }
    //SIGUSR1
    act.sa_sigaction = SIGUSR1_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    if(sigaction(SIGUSR1, &act, NULL) != 0){
        perror("blad przy ustawianiu handlera SIGUSR1 dla rodzica");
        exit(1);
    }
    //SIGCHLD
    act.sa_handler = SIGCHLD_support;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &act, NULL) != 0){
        perror("blad przu ustawianiu handlera SIGCHLD dla rodzica");
        exit(1);
    }

    //Utworzenie mask
    //  maska dziecka
    if(sigfillset(&child_mask) != 0){
        perror("blad przy tworzeniu maski sygnalow");
        exit(1);
    }
    if(sigdelset(&child_mask, SIGCONT) != 0){
        perror("blad przy usuwaniu SIGCONT z maski sygnalow");
        exit(1);
    }    
    //  maska pelna
    if(sigfillset(&full_mask) != 0){
        perror("blad przy tworzeniu maski zakrywajacej wszystkie procesy");
        exit(1);
    }

    //weryfikacja liczby argumentow
    if(argc != 3){
        printf("zla liczba argumentow, uzycie:\n./main liczbaPotomkow liczbaProsb\n");
        exit(1);
    }

    N = atoi(argv[1]);
    K = atoi(argv[2]) - 1;
    children = calloc(N, sizeof(pid_t));
    waiting_children = calloc(N, sizeof(pid_t));
    working = calloc(N, sizeof(int));

    //utworzenie N dzieci zmieniajac dla nich seed
    for(int i = 0; i < N; i++){
        srand(rand() % 1000);
        create_child(i);
    }

    while(children_finished != N){
        pause();
    }

    exit(0);
}