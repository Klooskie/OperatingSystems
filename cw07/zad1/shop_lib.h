#ifndef SHOP_LIB_H
#define SHOP_LIB_H

//stala i funkcja do tworzenia klucza IPC
#define KEY_GENERATOR 5
int generate_key();

//unia potrzebna do uzywania semctl
union semun{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
    struct seminfo * __budf;
};

//indeksy poszczegolnych semaforow
#define SEM_QUEUE 0
#define SEM_WAKE_UP 1
#define SEM_WOKEN 2
#define SEM_READY_FOR_SHAVING 3
#define SEM_SHAVED 4
#define SEM_LEFT 5

//funkcje do obslugi semaforow
void semaphore_take(int sem_set_id, int sem_index);
void semaphore_give(int sem_set_id, int sem_index);

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