#ifndef CONSTANTS_H
#define CONSTANTS_H

//nazwa klejki serwera
#define SERVER_QUEUE_NAME "/serverQ"

//limity
#define CLIENTS_NUMBER_LIMIT 5
#define MESSAGE_LENGTH_LIMIT 100
#define MESSAGE_NUMBER_LIMIT 10

//struktura wiadomosci
typedef struct message message;
struct message{
    long type;
    pid_t sender_pid;
    char mes[MESSAGE_LENGTH_LIMIT];
};

//typy komunikatow wysylanych do zerwera
#define MES_TYPE_INIT 1
#define MES_TYPE_MIRROR 2
#define MES_TYPE_CALC 3
#define MES_TYPE_TIME 4
#define MES_TYPE_END 5
#define MES_TYPE_STOP 6

//typ komunikatu wysylanego z serwera
#define MES_TYPE_RE 7

#endif