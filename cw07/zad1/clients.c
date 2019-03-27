#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "shop_lib.h"

int number_of_shaves;
int sem_set_id;
int sh_mem_id;
int key;
barber_shop * bshop;

void get_shaved(){

    semaphore_take(sem_set_id, SEM_QUEUE);

    if(bshop -> status != STATUS_AWAKE){
        //jesli golibroda spi to go obudz
        print_current_time();
        printf("Klient (%d) budzi golibrode\n", getpid());
        semaphore_give(sem_set_id, SEM_WAKE_UP);
        
        //zaczekaj na sygnal, ze jest juz obudzony
        semaphore_take(sem_set_id, SEM_WOKEN);
    }
    else{
        //jesli golibroda nie spi to zajmij miejsce w kolejce
        if(bshop -> number_of_occupied_seats < bshop -> number_of_seats){
            
            //jesli jest miejsce w kolejce to zajmij je
            print_current_time();
            printf("Klient (%d) zajmuje miejsce w poczekalni\n", getpid());
            sit_in_queue(bshop, getpid());
            semaphore_give(sem_set_id, SEM_QUEUE);

            //czekaj na sygnal od golibrody
            //ustworz maske sygnalow, tak by odblokowywala tylko sygnal SIGUSR1
            sigset_t sig_mask;
            sigfillset(&sig_mask);
            sigdelset(&sig_mask, SIGUSR1);
            sigsuspend(&sig_mask);            

            //zresetowanie flagi i oczekiwanie na semafor i opuszczenie kolejki
            semaphore_take(sem_set_id, SEM_QUEUE);
            leave_queue(bshop);
        }
        else{
            //jesli nie ma miejsca w kolejce, wyjdz
            print_current_time();
            printf("Klient (%d) opuszcza zaklad z powodu braku wolnych miejsc\n", getpid());
            semaphore_give(sem_set_id, SEM_QUEUE);
            get_shaved();
            return;
        }
    }

    //obudzilismy golibrode lub zaczekalismy na swoja kolej, siadamy na krzesle
    print_current_time();
    printf("Klient (%d) siada na krzesle\n", getpid());
    bshop -> current_client_pid = getpid();

    //zwiekszamy semafor aby powiadomic ze siedzimy na krzesle i oddajemy semafor odpowiedzialny za zarzadzanie kolejka
    semaphore_give(sem_set_id, SEM_READY_FOR_SHAVING);
    semaphore_give(sem_set_id, SEM_QUEUE);

    //oczekujemy na powiadomoienie od golibrody ze jestesmy ogoleni
    semaphore_take(sem_set_id, SEM_SHAVED);
    print_current_time();
    printf("Klient (%d) opuszcza zaklad po zakonczeniu strzyzenia\n", getpid());

    //powiadomienie golibrody ze wyszlismy
    semaphore_give(sem_set_id, SEM_LEFT);

}

void wait_for_clients(){

    pid_t child_pid = wait(NULL);
    while(child_pid != -1){   
        child_pid = wait(NULL);
    }

}

void init_shared_memory(){
    //uzyskanie identyfikatora pamieci wspoldzielonej
    if((sh_mem_id = shmget(key, 0, 0)) == -1){
        perror("blad przy uzywaniu shmget");
        exit(1);
    }

    //dolaczenie pamieci wspoldzielonej do wspolnej przestrzeni adresowej procesu
    if((bshop = (barber_shop *) shmat(sh_mem_id, NULL, 0)) == (void *) -1){
        perror("blad przy uzywaniu shmat");
        exit(1);
    }
}

void init_semaphores(){
    //uzyskanie identyfikatora zbioru semaforow
    if((sem_set_id = semget(key, 0, 0)) == -1){
        perror("blad przy uzywaniu semget");
        exit(1);
    }
}

void SIGUSR1_handler(int signo){
    //nie rob nic
}

void exit_handler(){
    //odlaczenie segmantu pamieci
    if(shmdt(bshop) == -1){
        perror("blad przy odlaczaniu segmentu pamieci wspoldzielonej");
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
    //wygenerowanie klucza zbioru semaforow i pamieci wspoldzielonej
    key = generate_key();

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