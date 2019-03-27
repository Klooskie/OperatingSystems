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
#include <semaphore.h>
#include <fcntl.h>
#include "shop_lib.h"

void print_current_time(){
    struct timespec * tp = malloc(sizeof(tp));
    if(clock_gettime(CLOCK_REALTIME, tp) != 0){
        perror("blad przy czytaniu aktualnego czasu");
        exit(1);
    }

    long int hours = (tp -> tv_sec / 3600) % 24;
    long int minutes = (tp -> tv_sec / 60) % 60;
    long int seconds = tp -> tv_sec % 60;
    long int mseconds = tp -> tv_nsec / 1000;

    printf("\033[1;32m%02ld:%02ld:%02ld:%06ld\033[0m -> ", hours, minutes, seconds, mseconds);

    free(tp);
}

void init_barber_shop(barber_shop * bshop, int number_of_seats){
    bshop -> number_of_seats = number_of_seats;
    bshop -> number_of_occupied_seats = 0;
    bshop -> first = 0;
    bshop -> status = STATUS_AWAKE;
}

void sit_in_queue(barber_shop * bshop, int pid){
    int index = (bshop -> first + bshop -> number_of_occupied_seats) % bshop -> number_of_seats;
    bshop -> queue[index] = pid;
    bshop -> number_of_occupied_seats += 1;
}

void leave_queue(barber_shop * bshop){
    bshop -> first = (bshop -> first + 1) % bshop -> number_of_seats;
    bshop -> number_of_occupied_seats -= 1;
}

void semaphore_take(sem_t * semaphore){
    if(sem_wait(semaphore) != 0){
        perror("blad przy uzywaniu sem_wait");
        exit(1);
    }
}

void semaphore_give(sem_t * semaphore){
    if(sem_post(semaphore) != 0){
        perror("blad przy uzywaniu sem_post");
        exit(1);
    }
}
