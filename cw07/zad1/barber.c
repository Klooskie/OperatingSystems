#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "shop_lib.h"

int number_of_seats;
int sem_set_id;
int sh_mem_id;
int key;
barber_shop * bshop;

void init_shared_memory(){
    //utworzenie pamieci wspoldzielonej
    if((sh_mem_id = shmget(key, sizeof(barber_shop), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1){
        perror("blad przy uzywaniu shmget");
        exit(1);
    }

    //dolaczenie pamieci wspoldzielonej do wspolnej przestrzeni adresowej procesu
    if((bshop = (barber_shop *) shmat(sh_mem_id, NULL, 0)) == (void *) -1){
        perror("blad przy uzywaniu shmat");
        exit(1);
    }

    init_barber_shop(bshop, number_of_seats);
}

void init_semaphores(){
    //utworzenie zbioru semaforow
    if((sem_set_id = semget(key, 6, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR)) == -1){
        perror("blad przy uzywaniu semget");
        exit(1);
    }

    //zainicjalizowanie poszczegolnych semaforow
    union semun arg;
    arg.val = 1;
    if(semctl(sem_set_id, SEM_QUEUE, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }

    arg.val = 0;
    if(semctl(sem_set_id, SEM_WAKE_UP, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }

    arg.val = 0;
    if(semctl(sem_set_id, SEM_WOKEN, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }

    arg.val = 0;
    if(semctl(sem_set_id, SEM_READY_FOR_SHAVING, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }
    
    arg.val = 0;
    if(semctl(sem_set_id, SEM_SHAVED, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }

    arg.val = 0;
    if(semctl(sem_set_id, SEM_LEFT, SETVAL, arg) == -1){
        perror("blad przy uzywaniu semctl");
        exit(1);
    }
}

void loop(){
    while(1){
        semaphore_take(sem_set_id, SEM_QUEUE);

        //sprawdzenie czy sa oczekujacy klienci
        if(bshop -> number_of_occupied_seats == 0){
            //jesli nikt nie czeka to zasypiamy i czekamy na to az ktos usiadzie na krzesle
            print_current_time();
            printf("Golibroda zasypia\n");
            bshop -> status = STATUS_ASLEEP;

            //oddanie semafora odpowiedzialnego za kolejke
            semaphore_give(sem_set_id, SEM_QUEUE);

            //oczekiwanie na obudzenie
            semaphore_take(sem_set_id, SEM_WAKE_UP);

            //zmiana statusu i powiadomienie o obudzeniu
            print_current_time();
            printf("Golibroda został obudzony\n");
            bshop -> status = STATUS_AWAKE;
            semaphore_give(sem_set_id, SEM_WOKEN);

        }
        else{
            //zaproszenie pierwszego klienta z poczekalni
            int client_pid = bshop -> queue[bshop -> first];
            kill(client_pid, SIGUSR1);
            print_current_time();
            printf("Golibroda zaprosil z kolejki klienta o pid %d\n", client_pid);

            //oddanie semafora odpowiedzialnego za kolejke
            semaphore_give(sem_set_id, SEM_QUEUE);           
        }

        //oczekiwanie az klient usiadzie na krzesle
        semaphore_take(sem_set_id, SEM_READY_FOR_SHAVING);

        //klient siedzi na krzesle, strzyzemy
        print_current_time();
        printf("Golibroda rozpoczyna strzyzenie klienta o pid %d\n", bshop -> current_client_pid);

        //konczymy strzyzenie
        print_current_time();
        printf("Golibroda zakonczyl strzyzenie klienta o pid %d\n", bshop -> current_client_pid);

        //powiadamiamy klienta o ostrzyzeniu i czekamy, az wyjdzie
        semaphore_give(sem_set_id, SEM_SHAVED);
        semaphore_take(sem_set_id, SEM_LEFT);
    
    }
}

void exit_handler(){
    //odlaczenie segmantu pamieci
    if(shmdt(bshop) == -1){
        perror("blad przy odlaczaniu segmentu pamieci wspoldzielonej");
    }

    //usuniecie pamieci wspoldzielonej
    if(shmctl(sh_mem_id, IPC_RMID, NULL) == -1){
        perror("blad przy usuwaniu pamieci wspoldzielonej");
    }
    
    //usuniecie zbioru semaforow
    if(semctl(sem_set_id, 0, IPC_RMID) == -1){
        perror("blad przy usuwaniu semaforow");
    }

    printf("wszystko pozamykano\n");
}

void SIGTERM_handler(int signo){
    printf("\notrzymano sygnal SIGTERM, zamykanie\n");
    exit(0);
}

void SIGINT_handler(int signo){
    printf("\notrzymano sygnal SIGINT, zamykanie\n");
    exit(0);
}

int main(int argc, char ** argv){
    
    //sprawdzenie liczby argumentow
    if(argc != 2){
        printf("zla liczba argumentow, uzycie:\n ./barber liczbaKrzesel\n");
        exit(1);
    }

    //ustawienie obslugi sygnalu SIGTERM
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = SIGTERM_handler;
    if(sigaction(SIGTERM, &sigact, NULL) < 0){
        perror("blad przy uzywaniu sigaction");
        exit(1);
    }

    //ustawienie obslugi sygnalu SIGINT takiej samej jak SIGTERM dla ulatwienia obslugi
    struct sigaction sact;
    sigemptyset(&sact.sa_mask);
    sact.sa_handler = SIGINT_handler;
    if(sigaction(SIGINT, &sact, NULL) < 0){
        perror("blad przy uzywaniu sigaction");
        exit(1);
    }

    //ustawienie funkcji zakonczenia
    if(atexit(exit_handler) != 0){
        perror("blad przy uzywaniu atexit");
        exit(1);
    }
    
    //zapamietanie ilosci miejsc w koljece
    number_of_seats = atoi(argv[1]);

    //wygenerowanie klucza zbioru semaforow i pamieci wspoldzielonej
    key = generate_key();

    //ustawienie pamieci wspoldzielonej
    init_shared_memory();

    //ustawienie semaforow
    init_semaphores();

    //wykonywanie glownej petli programu
    loop();
}