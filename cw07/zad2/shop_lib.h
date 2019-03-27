#ifndef SHOP_LIB_H
#define SHOP_LIB_H

//stala i funkcja do tworzenia klucza IPC
#define KEY_NAME "/shop"

//unia potrzebna do uzywania semctl
union semun{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
    struct seminfo * __budf;
};

//indeksy poszczegolnych semaforow
#define SEM_QUEUE "/semQueue"
#define SEM_WAKE_UP "/semWakeUp"
#define SEM_WOKEN "/semWoken"
#define SEM_READY_FOR_SHAVING "/semReadyForShaving"
#define SEM_SHAVED "/semShaved"
#define SEM_LEFT "/semLeft"

//funkcje do obslugi semaforow
void semaphore_take(sem_t * semaphore);
void semaphore_give(sem_t * semaphore);

//maksymalny rozmiar kolejki
#define MAX_QUEUE_SIZE 30

//struktura przechowywana w pamieci wspoldzielonej
typedef struct barber_shop barber_shop;
struct barber_shop{
    //pola od ktorych zalezy obsluga kolejki
    int queue[MAX_QUEUE_SIZE];
    int number_of_seats;
    int number_of_occupied_seats;
    int first;
    int status;
    //pole od ktorego zalezy obsluga konkretnego klienta
    int current_client_pid;
};

//stale definijace status barbera
#define STATUS_AWAKE 1
#define STATUS_ASLEEP 2

//funkcje do obslugi kolejki
void init_barber_shop(barber_shop * bshop, int number_of_seats);
void sit_in_queue(barber_shop * bshop, int pid);
void leave_queue(barber_shop * bshop);

//funkcja do formatowania aktualnego czasu
void print_current_time();

#endif