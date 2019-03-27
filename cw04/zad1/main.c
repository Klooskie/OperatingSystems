#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

int run = 1;

void SIGINTsupport(int signum){
    printf("\nOdebrano sygnal SIGINT(%d)\n", signum);
    exit(0);
}

void SIGTSTPsupport(int signum){
    if(run == 1)
        printf("\nOtrzymano sygnal SIGTSTP(%d)\nOczekue na CTRL+Z-kontynuacja albo CTRL+C-zakonczenie programu\n", signum);
    else
        printf("\n");
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

    while(1){
        time_t current_time = time(NULL);
        printf("%s", ctime(&current_time));
        sleep(1);

        if(run == -1){
            pause();
        }
    }
}