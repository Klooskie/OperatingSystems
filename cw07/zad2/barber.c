#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "shop_lib.h"

int number_of_seats;
sem_t * sem_queue;
sem_t * sem_wake_up;
sem_t * sem_woken;
sem_t * sem_ready_for_shaving;
sem_t * sem_shaved;
sem_t * sem_left;
barber_shop * bshop;

void init_shared_memory(){
    //utworzenie pamieci wspolzielonej
    int descriptor = shm_open(KEY_NAME, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if(descriptor == -1){
        perror("blad przy tworzeniu segmentu pamieci wspoldzielonej");
        exit(1);
    }

    //okreslenie rozmiaru pamieci wspoldzielonej
    if(ftruncate(descriptor, sizeof(barber_shop)) != 0){
        perror("blad przy okreslaniu rozmiaru pamieci wspoldzielonej");
        exit(1);
    }    

    //dolaczenie pamieci do prrzestrzeni adresowej procesu
    bshop = (barber_shop *) mmap(NULL, sizeof(barber_shop), PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if(bshop == (void *) -1){
        perror("blad przy uzywaniu mmap");
        exit(1);
    }

    init_barber_shop(bshop, number_of_seats);
}

void init_semaphores(){
    
    sem_queue = sem_open(SEM_QUEUE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
    if(sem_queue == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_queue");
        exit(1);
    }
   
    sem_wake_up = sem_open(SEM_WAKE_UP, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(sem_wake_up == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_wake_up");
        exit(1);
    }

    sem_woken = sem_open(SEM_WOKEN, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(sem_woken == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_woken");
        exit(1);
    }

    sem_ready_for_shaving = sem_open(SEM_READY_FOR_SHAVING, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(sem_ready_for_shaving == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_ready_for_shaving");
        exit(1);
    }

    sem_shaved = sem_open(SEM_SHAVED, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(sem_shaved == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_shaved");
        exit(1);
    }

    sem_left = sem_open(SEM_LEFT, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0);
    if(sem_left == SEM_FAILED){
        perror("blad przy tworzeniu semafora sem_left");
        exit(1);
    }
}

void loop(){
    while(1){
        semaphore_take(sem_queue);

        //sprawdzenie czy sa oczekujacy klienci
        if(bshop -> number_of_occupied_seats == 0){
            //jesli nikt nie czeka to zasypiamy i czekamy na to az ktos usiadzie na krzesle
            print_current_time();
            printf("Golibroda zasypia\n");
            bshop -> status = STATUS_ASLEEP;

            //oddanie semafora odpowiedzialnego za kolejke
            semaphore_give(sem_queue);

            //oczekiwanie na obudzenie
            semaphore_take(sem_wake_up);

            //zmiana statusu i powiadomienie o obudzeniu
            print_current_time();
            printf("Golibroda został obudzony\n");
            bshop -> status = STATUS_AWAKE;
            semaphore_give(sem_woken);

        }
        else{
            //zaproszenie pierwszego klienta z poczekalni
            int client_pid = bshop -> queue[bshop -> first];
            kill(client_pid, SIGUSR1);
            print_current_time();
            printf("Golibroda zaprosil z kolejki klienta o pid %d\n", client_pid);

            //oddanie semafora odpowiedzialnego za kolejke
            semaphore_give(sem_queue);           
        }

        //oczekiwanie az klient usiadzie na krzesle
        semaphore_take(sem_ready_for_shaving);

        //klient siedzi na krzesle, strzyzemy
        print_current_time();
        printf("Golibroda rozpoczyna strzyzenie klienta o pid %d\n", bshop -> current_client_pid);

        //konczymy strzyzenie
        print_current_time();
        printf("Golibroda zakonczyl strzyzenie klienta o pid %d\n", bshop -> current_client_pid);

        //powiadamiamy klienta o ostrzyzeniu i czekamy, az wyjdzie
        semaphore_give(sem_shaved);
        semaphore_take(sem_left);
    
    }
}

void exit_handler(){
    //odlaczenie segmantu pamieci
    if(munmap(bshop, sizeof(barber_shop)) != 0){
        perror("blad przy odlaczaniu segmentu pamieci wspoldzielonej");
    }

    //usuniecie pamieci wspoldzielonej
    if(shm_unlink(KEY_NAME) != 0){
        perror("blad przy usuwaniu pamieci wspoldzielonej");
    }
    
    //usuniecie semaforow
    if(sem_close(sem_queue) != 0){
        perror("blad przy zamykaniu sem_queue");
    }
    if(sem_unlink(SEM_QUEUE) != 0){
        perror("blad przy usuwaniu sem_queue");
    }

    if(sem_close(sem_wake_up) != 0){
        perror("blad przy zamykaniu sem_wake_up");
    }
    if(sem_unlink(SEM_WAKE_UP) != 0){
        perror("blad przy usuwaniu sem_wake_up");
    }

    if(sem_close(sem_woken) != 0){
        perror("blad przy zamykaniu sem_woken");
    }
    if(sem_unlink(SEM_WOKEN) != 0){
        perror("blad przy usuwaniu sem_woken");
    }

    if(sem_close(sem_ready_for_shaving) != 0){
        perror("blad przy zamykaniu sem_ready_for_shaving");
    }
    if(sem_unlink(SEM_READY_FOR_SHAVING) != 0){
        perror("blad przy usuwaniu sem_ready_for_shaving");
    }

    if(sem_close(sem_shaved) != 0){
        perror("blad przy zamykaniu sem_shaved");
    }
    if(sem_unlink(SEM_SHAVED) != 0){
        perror("blad przy usuwaniu sem_shaved");
    }

    if(sem_close(sem_left) != 0){
        perror("blad przy zamykaniu sem_left");
    }
    if(sem_unlink(SEM_LEFT) != 0){
        perror("blad przy usuwaniu sem_left");
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

    //ustawienie pamieci wspoldzielonej
    init_shared_memory();

    //ustawienie semaforow
    init_semaphores();

    //wykonywanie glownej petli programu
    loop();
}