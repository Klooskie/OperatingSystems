#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <fcntl.h>
#include "constants.h"

int server_qid = -1;
int client_qid = -1;
char * queue_name;

void send_message(int type, char * mes){
    //obsluga za dlugich wiadomosci
    if(mes != NULL)
        if(strlen(mes) >= MESSAGE_LENGTH_LIMIT){
            printf("wiadomosc przekroczyla limit dlugosci - %d znakow\n", MESSAGE_LENGTH_LIMIT);
            return;
        }

    //utworzenie wiadomosci
  /*  message m;
    m.type = type;
    m.sender_pid = getpid();
    if(mes != NULL)
        strcpy(m.mes, mes);
    else
        strcpy(m.mes, "-");
*/
    char m[MESSAGE_LENGTH_LIMIT];
    if(mes != NULL)
        sprintf(m, "%d %d %s", type, getpid(), mes);
    else    
        sprintf(m, "%d %d %s", type, getpid(), "-");

    //wyslanie wiadomosci
    if(mq_send(server_qid, m, MESSAGE_LENGTH_LIMIT, 1) != 0){
        perror("blad przy wysylaniu komunikatu do serwera");
        exit(1);
    }
}

message receive_message(){
    //odebranie wiadomosci
    char m[MESSAGE_LENGTH_LIMIT];    
    if(mq_receive(client_qid, m, MESSAGE_LENGTH_LIMIT, NULL) == -1){
        perror("blad przy odbieraniu komunikatu");
        exit(1);
    }
    
    //konwersja wiadomosci do struktury message
    message me;
    me.type = atoi(strtok(m, " "));
    me.sender_pid = atoi(strtok(NULL, " "));
    strcpy(me.mes, strtok(NULL, "\0"));

    return me;
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
    //wyslanie wiadomosci do serwera
    send_message(MES_TYPE_INIT, queue_name);

    //oczekiwanie na odpowiedz
    message m = receive_message();

    //obsluzenie odpowiedzi err
    if(strcmp(m.mes, "err") == 0){
        printf("serwer jest juz przepelniony\nzamkniecie klienta\n");
        server_qid = -1;
        if(mq_close(server_qid) != 0){
            perror("blad przy zamykaniu kolejki serwera");
        }
        exit(0);
    }
}

void get_server_queue_id(){
    //otwarcie kolejki serwera
    server_qid = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if(server_qid == -1){
        perror("blad przy otwieraniu kolejki serwera");
        exit(1);
    }
}

void init_queue(){

    //ustalenie nazwy na kolejke klienta
    queue_name = malloc(30);
    queue_name[0] = '/';
    srand(time(NULL));
    for(int i = 1; i < 30; i++){
        queue_name[i] = (char)((rand() % 26) + 97);
    }

    printf("nazwa kolejki %s\n", queue_name);

    //ustawienie atrybutow kolejki
    struct mq_attr * attribute = malloc(sizeof(struct mq_attr));
    attribute -> mq_msgsize = MESSAGE_LENGTH_LIMIT;
    attribute -> mq_maxmsg = MESSAGE_NUMBER_LIMIT;

    //stworzenie kolejki
    client_qid = mq_open(queue_name, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, attribute);
    if(client_qid == -1){
        perror("blad przy tworzeniu kolejki klienta");
        exit(1);
    }
}

void exit_fun(){
    
    //zamkniecie kolejki serwera
    if(server_qid != -1){
        //wyslanie komunikatu STOP jesli klient jest podlaczony do serwera
        send_message(MES_TYPE_STOP, "-");

        if(mq_close(server_qid) != 0){
            perror("blad przy zamykaniu kolejki serwera");
        }
    }

    //zamkniecie i usuniecie kolejki klienta
    if(client_qid != -1){
        if(mq_close(client_qid) != 0){
            perror("blad przy zamykaniu kolejki klienta");
        }

        if(mq_unlink(queue_name) != 0){
            perror("blad przy usuwaniu kolejki klienta");
        }
    }
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