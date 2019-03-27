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

mqd_t queue_id = -1;
int clients_number = 0;
int clients_pids[CLIENTS_NUMBER_LIMIT];
mqd_t clients_queues[CLIENTS_NUMBER_LIMIT];
int exit_flag = 0;

void respond(pid_t pid, char * mes){
    //przygotowanie wiadomosci
 /*   message m;
    m.type = MES_TYPE_RE;
    m.sender_pid = getpid();
    strcpy(m.mes, mes);
*/
    char m[MESSAGE_LENGTH_LIMIT];
    sprintf(m, "%d %d %s", MES_TYPE_RE, getpid(), mes);

    //znalezienie id kolejki z danym klientem
    int qid = -1;
    for(int i = 0; i < clients_number; i++)
        if(clients_pids[i] == pid)
            qid = clients_queues[i];

    //wyslanie wiadomosci
    if(mq_send(qid, m, MESSAGE_LENGTH_LIMIT, 1) != 0){
        perror("blad przy wysylaniu odpowiedzi do klienta");
        exit(1);
    }
}

void init_message_support(message m){
    //sprawdzenie czy kolejny klient moze sie podlaczyc
    if(clients_number == CLIENTS_NUMBER_LIMIT){
        //jesli nie, wypisanie komunikatu i kontynuacja pracy serwera
        printf("klient o id %d nie moze sie polaczyc, serwer pelny\n", m.sender_pid);        
        /*
        message me;
        me.type = MES_TYPE_RE;
        me.sender_pid = getpid();
        strcpy(me.mes, "err");
        */
        //wyslanie wiadomosci zwrotnej
        char me[MESSAGE_LENGTH_LIMIT];
        sprintf(me, "%d %d %s", MES_TYPE_RE, getpid(), "err");

        if(mq_send(atoi(m.mes), me, MESSAGE_LENGTH_LIMIT, 1) == -1){
            perror("blad przy wysylaniu odpowiedzi do klienta");
            exit(1);
        }

        return;
    }

    //zapisanie pid'u klienta i numeru kolejki, ktory wysyla w komunikacie
    clients_pids[clients_number] = m.sender_pid;
    clients_queues[clients_number] = atoi(m.mes);

    clients_number++;

    //wyslanie komunikatu do klienta
    respond(m.sender_pid, "ok");

    //komunikat o pomyslnym dolaczeniu klienta
    printf("klient podlaczyl sie do serwera - id %d, kolejka %d\n", m.sender_pid, atoi(m.mes));
}

void mirror_message_support(message m){
    
    //stworzenie lustrzanego odbicia wiadomosci z komunikatu w tablicy mes
    char * mes = calloc(MESSAGE_LENGTH_LIMIT, sizeof(char));
    int message_length = strlen(m.mes);
    for(int i = 0; i < message_length; i++){
        int index = message_length - i - 1;
        mes[i] = m.mes[index];
    }

    //wyslanie komunikatu do klienta
    respond(m.sender_pid, mes);

}

void calc_message_support(message m){
    
    char * mes = calloc(MESSAGE_LENGTH_LIMIT, sizeof(char));

    char * left_number = calloc(MESSAGE_LENGTH_LIMIT, 1);
    char * right_number = calloc(MESSAGE_LENGTH_LIMIT, 1);
    char operation;

    int li = 0, ri = 0, number = 0;
    int message_length = strlen(m.mes);
    for(int i = 0; i < message_length; i++){
        if(m.mes[i] >= 48 && m.mes[i] <= 57){
            if(number == 0){
                left_number[li] = m.mes[i];
                li++;
            }
            else{
                right_number[ri] = m.mes[i];
                ri++;
            }
        }
        else if(m.mes[i] == ' '){
            continue;
        }
        else if(m.mes[i] == '+' || m.mes[i] == '-' || m.mes[i] == '*' || m.mes[i] == '/'){
            if(number == 0){
                operation = m.mes[i];
                number = 1;
            }
            else{
                strcpy(mes, "bledna operacja");
                respond(m.sender_pid, mes);
                return;
            }
        }
        else{
            strcpy(mes, "bledna operacja");
            respond(m.sender_pid, mes);
            return;
        }
    }

    if(li == 0 || ri == 0){
        strcpy(mes, "bledna operacja");
        respond(m.sender_pid, mes);
        return;
    }

    int l = atoi(left_number);
    int r = atoi(right_number);
    if(operation == '/' && r == 0){
        strcpy(mes, "bledna operacja");
        respond(m.sender_pid, mes);
        return;
    }

    int result;
    if(operation == '+') result = l + r;
    if(operation == '-') result = l - r;
    if(operation == '*') result = l * r;
    if(operation == '/') result = l / r;

    sprintf(mes, "%d", result);
    respond(m.sender_pid, mes);

}

void time_message_support(message m){
    
    //zapamietanie w mes aktualnej daty
    time_t curtime = time(NULL);
    char * mes = ctime(&curtime);

    //wyslanie komunikatu do klienta
    respond(m.sender_pid, mes);

}

void end_message_support(message m){
    //ustawienie flagi exit_flag na 1, aby skonczyc dzialanie serwera w momencie odebrania wszystkich sygnalow z kolejki
    exit_flag = 1;
}

void stop_message_support(message m){
    //znalezienie kolejki klienta
    int qid = -1;
    int index;
    for(int i = 0; i < clients_number; i++)
        if(clients_pids[i] == m.sender_pid)
        {
            qid = clients_queues[i];
            index = i;
        }
    
    if(qid != -1){
        //zwolnienie miejsca na nowego klienta
        clients_number--;
        for(int i = index; i < clients_number; i++){
            clients_pids[i] = clients_pids[i+1];
            clients_queues[i] = clients_pids[i+1];
        }
    }
}

void handle_message(message m){
    if(m.type == MES_TYPE_INIT)
        init_message_support(m);

    if(m.type == MES_TYPE_STOP)
        stop_message_support(m);

    if(m.type == MES_TYPE_MIRROR)
        mirror_message_support(m);

    if(m.type == MES_TYPE_CALC)
        calc_message_support(m);

    if(m.type == MES_TYPE_TIME)
        time_message_support(m);

    if(m.type == MES_TYPE_END)
        end_message_support(m);
}

void wait_for_message(){


    //odczytanie wiadomosci z koljeki
    char me[MESSAGE_LENGTH_LIMIT];
    //odczytanie wiadomosci zaczynajac od 1. pola struktury po prostu zadziala jak odczytanie struktury
    if(mq_receive(queue_id, me, MESSAGE_LENGTH_LIMIT, NULL) < 0){
        perror("blad podczas odbierania wiadomosci z kolejki serwera");
        exit(1);
    }

    message m;
    m.type = atoi(strtok(me, " "));
    m.sender_pid = atoi(strtok(NULL, " "));
    strcpy(m.mes, strtok(NULL, "\0"));

    //wypisanie komunikatu o otrzymanej wiadomosci
    printf("\x1b[32motrzymano wiadomosc:\x1b[0m typ %ld, od %d, tresc %s\n", m.type, m.sender_pid, m.mes);

    handle_message(m);

}

void init_queue(){
    //ustawienie atrybutow kolejki
    struct mq_attr * attribute = malloc(sizeof(struct mq_attr));
    attribute -> mq_msgsize = MESSAGE_LENGTH_LIMIT;
    attribute -> mq_maxmsg = MESSAGE_NUMBER_LIMIT;

    //stworzenie kolejki
    queue_id = mq_open(SERVER_QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR,  S_IRUSR | S_IWUSR, attribute);
    if(queue_id == -1){
        perror("blad przy tworzeniu kolejki serwera");
        exit(1);
    }
}

void exit_fun(){
    //zamkniecie wszyskich koljek klientow
    for(int i = 0; i < clients_number; i ++){
        if(clients_queues[i] != -1){
            if(mq_close(clients_queues[i]) != 0)
                perror("blad przy zamykaniu kolejki klienta");
            clients_queues[i] = -1;
        }
    }

    //zamkniecie kolejki serwera
    if(queue_id != -1){
        if(mq_close(queue_id) != 0){
            perror("blad przy zamykaniu kolejki serwera");
        }
    }

    //usuniecie kolejki serwera
    if(mq_unlink(SERVER_QUEUE_NAME) != 0){
        perror("blad przy usuwaniu kolejki serwera");
    }

}

void SIGINT_handler(int sig_no){
    printf("zamykanie serwera\n");
    exit(1);
}

int main(void){
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

    //tworzenie kolejki
    init_queue();

    printf("serwer utworzony pomyslnie\n");
    printf("deskryptor kolejki: %d\n", queue_id);

    //oczekiwanie na komunikaty
    while(1){
        wait_for_message();
        if(exit_flag == 1){
            printf("zamykanie serwera\n");
            exit(0);
        }
    }

}