#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int run = -1;
int pid;

void SIGINTsupport(int signum){
    
    printf("\nOdebrano sygnal SIGINT(%d)\n", signum);
    
    //zabicie procesu potomnego
    if(run == 1){
        if(kill(pid, SIGKILL) != 0){
            perror("blad przy zabijaniu procesu potomnego");
            exit(1);
        }
        printf("Zabito proces potomny\n");
    }   
    //zakonczenie programu
    exit(0);

}

void SIGTSTPsupport(int signum){
    if(run == 1){
        //obsluga przypadku kiedy proces potomny juz pracuje
        printf("\nOtrzymano sygnal SIGTSTP(%d)\n", signum);
        if(kill(pid, SIGKILL) != 0){
            perror("blad przy zabijaniu procesu potomnego");
            exit(1);
        }
        printf("Zabito procespotomny\nOczekue na CTRL+Z-kontynuacja albo CTRL+C-zakonczenie programu\n");
    }
    else{
        //obsluga przypadku kiedy nie ma procesu potomnego
        printf("\n");
        pid = fork();
        if(pid < 0){
            perror("blad przy tworzeniu nowego procesu");
            exit(1);
        }
        else if(pid == 0){
            execl("script.sh", "script", NULL);
        }
        else
            printf("PID potomka: %d\n", pid);
    }

    run *= -1;
}

int main(int argc, char ** argv){
    //ustawienie obslugi sygnalu SIGINT
    signal(SIGINT, SIGINTsupport);

    //ustawienie obslugi sygnalu SIGTSTP
    struct sigaction * act = malloc(sizeof(struct sigaction));
    act -> sa_handler = SIGTSTPsupport;
    sigemptyset(&act -> sa_mask);
    act -> sa_flags = 0;
    sigaction(SIGTSTP, act, NULL);

    SIGTSTPsupport(0);
    while(1){
        pause();
    }
}