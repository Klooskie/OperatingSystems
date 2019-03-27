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

int number_of_shaves;
sem_t * sem_queue;
sem_t * sem_wake_up;
sem_t * sem_woken;
sem_t * sem_ready_for_shaving;
sem_t * sem_shaved;
sem_t * sem_left;
barber_shop * bshop;

void get_shaved(){

    semaphore_take(sem_queue);

    if(bshop -> status != STATUS_AWAKE){
        //jesli golibroda spi to go obudz
        print_current_time();
        printf("Klient (%d) budzi golibrode\n", getpid());
        semaphore_give(sem_wake_up);
        
        //zaczekaj na sygnal, ze jest juz obudzony
        semaphore_take(sem_woken);
    }
    else{
        //jesli golibroda nie spi to zajmij miejsce w kolejce
        if(bshop -> number_of_occupied_seats < bshop -> number_of_seats){
            
            //jesli jest miejsce w kolejce to zajmij je
            print_current_time();
            printf("Klient (%d) zajmuje miejsce w poczekalni\n", getpid());
            sit_in_queue(bshop, getpid());
            semaphore_give(sem_queue);

            //czekaj na sygnal od golibrody
            //ustworz maske sygnalow, tak by odblokowywala tylko sygnal SIGUSR1
            sigset_t sig_mask;
            sigfillset(&sig_mask);
            sigdelset(&sig_mask, SIGUSR1);
            sigsuspend(&sig_mask);            

            //zresetowanie flagi i oczekiwanie na semafor i opuszczenie kolejki
            semaphore_take(sem_queue);
            leave_queue(bshop);
        }
        else{
            //jesli nie ma miejsca w kolejce, wyjdz
            print_current_time();
            printf("Klient (%d) opuszcza zaklad z powodu braku wolnych miejsc\n", getpid());
            semaphore_give(sem_queue);
            get_shaved();
            return;
        }
    }

    //obudzilismy golibrode lub zaczekalismy na swoja kolej, siadamy na krzesle
    print_current_time();
    printf("Klient (%d) siada na krzesle\n", getpid());
    bshop -> current_client_pid = getpid();

    //zwiekszamy semafor aby powiadomic ze siedzimy na krzesle i oddajemy semafor odpowiedzialny za zarzadzanie kolejka
    semaphore_give(sem_ready_for_shaving);
    semaphore_give(sem_queue);

    //oczekujemy na powiadomoienie od golibrody ze jestesmy ogoleni
    semaphore_take(sem_shaved);
    print_current_time();
    printf("Klient (%d) opuszcza zaklad po zakonczeniu strzyzenia\n", getpid());

    //powiadomienie golibrody ze wyszlismy
    semaphore_give(sem_left);

}

void wait_for_clients(){

    pid_t child_pid = wait(NULL);
    while(child_pid != -1){   
        child_pid = wait(NULL);
    }

}

void init_shared_memory(){
    //otwarcie segmentu pamieci wspoldzielonej
    int descriptor = shm_open(KEY_NAME, O_RDWR, 0);
    if(descriptor == -1){
        perror("blad przy tworzeniu segmentu pamieci wspoldzielonej");
        exit(1);
    }

    //dolaczenie pamieci do prrzestrzeni adresowej procesu
    bshop = (barber_shop *) mmap(NULL, sizeof(barber_shop), PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if(bshop == (void *) -1){
        perror("blad przy uzywaniu mmap");
        exit(1);
    }
}

void init_semaphores(){
    //otwarcie kolejnych semaforow
    sem_queue = sem_open(SEM_QUEUE, 0);
    if(sem_queue == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_queue");
        exit(1);
    }

    sem_wake_up = sem_open(SEM_WAKE_UP, 0);
    if(sem_wake_up == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_wake_up");
        exit(1);
    }

    sem_woken = sem_open(SEM_WOKEN, 0);
    if(sem_woken == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_woken");
        exit(1);
    }

    sem_ready_for_shaving = sem_open(SEM_READY_FOR_SHAVING, 0);
    if(sem_ready_for_shaving == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_ready_for_shaving");
        exit(1);
    }

    sem_shaved = sem_open(SEM_SHAVED, 0);
    if(sem_shaved == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_shaved");
        exit(1);
    }

    sem_left = sem_open(SEM_LEFT, 0);
    if(sem_left == SEM_FAILED){
        perror("blad przy otwieraniu semafora sem_left");
        exit(1);
    }
}

void SIGUSR1_handler(int signo){
    //nie rob nic
}

void exit_handler(){
    //odlaczenie segmantu pamieci
    if(munmap(bshop, sizeof(barber_shop)) != 0){
        perror("blad przy odlaczaniu segmentu pamieci wspoldzielonej");
    }

    //zamkniecie poszczegolnych semaforow
    if(sem_close(sem_queue) != 0){
        perror("blad przy zamykaniu sem_queue");
    }

    if(sem_close(sem_wake_up) != 0){
        perror("blad przy zamykaniu sem_wake_up");
    }

    if(sem_close(sem_woken) != 0){
        perror("blad przy zamykaniu sem_woken");
    }

    if(sem_close(sem_ready_for_shaving) != 0){
        perror("blad przy zamykaniu sem_ready_for_shaving");
    }

    if(sem_close(sem_shaved) != 0){
        perror("blad przy zamykaniu sem_shaved");
    }

    if(sem_close(sem_left) != 0){
        perror("blad przy zamykaniu sem_left");
    }

}

void block_SIGUSR1(){
    //ustawienie maski sygnalow tak, by blokowac SIGUSR1
    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGUSR1);
    sigprocmask(SIG_SETMASK, &sig_mask, NULL);
}

void set_SIGUSR1_handler(){
    //ustawienie obslugi sygnalu SIGUSR1
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_handler = SIGUSR1_handler;
    if(sigaction(SIGUSR1, &sigact, NULL) < 0){
        perror("blad przy uzywaniu sigaction");
        exit(1);
    }
}

void set_exit_fun(){
    //ustawienie funkcji zakonczenia
    if(atexit(exit_handler) != 0){
        perror("blad przy uzywaniu atexit");
        exit(1);
    }
}

void init_ipc(){
    //ustawienie pamieci wspoldzielonej
    init_shared_memory();

    //ustawienie semaforow
    init_semaphores();
}

void create_clients(int number_of_clients){

    for(int i = 0; i < number_of_clients; i++){

        //stworzenie potomka
        pid_t pid = fork();
        if(pid < 0){
            perror("blad przy tworzeniu potomka");
            exit(1);
        }
        else if(pid == 0){
            //zainicjalizowanie obslugi sygnalow, funkcji wyjscia i ipc
            block_SIGUSR1();
            set_SIGUSR1_handler();
            set_exit_fun();
            init_ipc();

            //odwiedzenie barbera number_of_shaves razy
            for(int j = 0; j < number_of_shaves; j++){
                get_shaved();
            }

            //zakonczenie potomka
            exit(0);
        }
    }
}

int main(int argc, char ** argv){
    //sprawdzenie liczby argumentow
    if(argc != 3){
        printf("zla liczba argumentow, uzycie:\n ./clients liczbaKlientow liczbaStrzyzen\n");
        exit(1);
    }

    //zapamietanie ilosci klientow i ilosci strzyzen
    int number_of_clients = atoi(argv[1]);
    number_of_shaves = atoi(argv[2]);

    //utworzenie number_of_clients potomkow
    create_clients(number_of_clients);

    //oczekiwanie na zakonczenie wszystkich potomkow
    wait_for_clients();

}