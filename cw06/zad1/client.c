#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "constants.h"

int server_qid = -1;
int client_qid = -1;

void send_message(int type, char * mes){
    //obsluga za dlugich wiadomosci
    if(mes != NULL)
        if(strlen(mes) >= MESSAGE_LENGTH_LIMIT){
            printf("wiadomosc przekroczyla limit dlugosci - %d znakow\n", MESSAGE_LENGTH_LIMIT);
            return;
        }

    //utworzenie wiadomosci
    message m;
    m.type = type;
    m.sender_pid = getpid();
    if(mes != NULL)
        strcpy(m.mes, mes);
    else
        strcpy(m.mes, "-");

    //wyslanie wiadomosci
    if(msgsnd(server_qid, &m, sizeof(message) - sizeof(long), 0) != 0){
        perror("blad przy wysylaniu komunikatu do serwera");
        exit(1);
    }
}

message receive_message(){
    message m;
    if(msgrcv(client_qid, &m, sizeof(message) - sizeof(long), MES_TYPE_RE, 0) == -1){
        perror("blad przy odbieraniu komunikatu");
        exit(1);
    }
    return m;
}

void read_line(){
    //odczytanie linii
    char * line = NULL;
    size_t size = 0;
    getline(&line, &size, stdin);

    //wyluskanie pierwszego slowa
    char * command = strtok(line, " \n");

    //uslanie odpowiedniego komunikatu w zaleznosci od komendy
    if(command == NULL)
        return;
    else if(strcmp(command, "MIRROR") == 0)    
        send_message(MES_TYPE_MIRROR, strtok(NULL, "\n"));
    else if(strcmp(command, "CALC") == 0)    
        send_message(MES_TYPE_CALC, strtok(NULL, "\n"));
    else if(strcmp(command, "TIME") == 0)    
        send_message(MES_TYPE_TIME, NULL);
    else if(strcmp(command, "END") == 0){
        send_message(MES_TYPE_END, NULL);
        server_qid = -1;
        exit(0);
    }
    else{
        printf("bledna komenda\n");
        return;
    }

    //oczekiwanie na odpowiedz
    message m = receive_message();
    printf("\x1b[32modpowiedz serwera:\x1b[0m\n%s", m.mes);
    if(strcmp(command, "TIME") != 0)
        printf("\n");

}

void init_connection(){
    //wyslanie wiadomosci
    char mes[MESSAGE_LENGTH_LIMIT];
    sprintf(mes, "%d ", client_qid);
    send_message(MES_TYPE_INIT, mes);

    //oczekiwanie na odpowiedz
    message m = receive_message();

    //obsluzenie odpowiedzi err
    if(strcmp(m.mes, "err") == 0){
        printf("serwer jest juz przepelniony\nzamkniecie klienta\n");
        server_qid = -1;
        exit(0);
    }
}

void get_server_queue_id(){
    //zapamietanie zmiennej srodowiskowej HOME
    char * path = getenv("HOME");

    //ustawienie klucza do stworzenia kolejki
    int key = ftok(path, KEY_GENERATOR);
    if(key == -1){
        perror("blad przy uzywaniu ftok");
        exit(1);
    }

    //stworzenie kolejki
    server_qid = msgget(key, 0);
    if(server_qid == -1){
        perror("blad przy zdobywaniu qid kolejki serwera");
        exit(1);
    }
}

void init_queue(){
    //stworzenie kolejki
    client_qid = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR);
    if(client_qid < 0){
        perror("blad przy tworzeniu kolejki klienta");
        exit(1);
    }
}

void exit_fun(){
    //wyslanie komunikatu STOP jesli klient jest podlaczony do serwera
    if(server_qid != -1)
        send_message(MES_TYPE_STOP, "-");

    if(client_qid != -1)
        if(msgctl(client_qid, IPC_RMID, NULL) != 0)
            perror("blad przy usuwaniu kolejki klienta");
}

void SIGINT_handler(){
    printf("zamykanie klienta\n");
    exit(0);
}

int main(){
     //ustawienie funkcji do wykonywania przy zakonczeniu programu
    if(atexit(exit_fun) != 0){
        perror("blad przy uzywaniu atexit");
        exit(1);
    }

    //ustawienie obslugi sygnalu SIGINT
    struct sigaction s;
    s.sa_handler = SIGINT_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = 0;
    if(sigaction(SIGINT, &s, NULL) != 0){
        perror("blad przu ustawianiu handlera SIGINT");
        exit(1);
    }

    //stworzenie kolejki klienta
    init_queue();

    //zdobycie qid kolejki serwera
    get_server_queue_id();

    //wyslanie komunikatu typu INIT i odebranie potwierdzenia
    init_connection();

    printf("klient utworzony pomysnlnie\n");
    printf("id kolejki prywatnej z serwerem: %d\n", client_qid);
    
    //odczytywanie komend klienta
    while(1){
        read_line();
    }
}