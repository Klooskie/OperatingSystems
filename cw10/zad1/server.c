#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "constants.h"


typedef struct client_data client_data;
struct client_data{
    int socket_fd;
    char name[50];
    int registered;
    int ponged;
};


char * unix_socket_path;
int port_number;

int socket_descriptor_inet;
int socket_descriptor_local;

int calculation_counter = 0;
int last_used_client = 0;
int close_flag = 0;

client_data clients_data[number_of_clients_limit];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void result_message_fun(int client_id, char * message){
    printf("Otrzymano wynik dzialania od %d - %s -> %s\n", client_id, clients_data[client_id].name, message);
}


void ping_message_fun(int client_id){
    clients_data[client_id].ponged = 1;
}


void registration_message_fun(int client_id, char * name){

    //skopiowanie nazwy klienta i zapamietanie statusu 
    strcpy(clients_data[client_id].name, name);
    clients_data[client_id].registered = 1;

    //wyslanie wiadomosci zwrotnej
    send_message(MESSAGE_ACCEPT, NULL, clients_data[client_id].socket_fd);
    printf("Zaakceptowano rejestracje od klienta id: %d - %s\n", client_id, name);

}


void receive_message(int client_id){
    //przeczytanie wiadomosci z gniazda
    msg message;
    get_message(&message, clients_data[client_id].socket_fd);

    //odpowiednie obsluzenie danego typu wiadomosci
    if(message.type == MESSAGE_REGISTER)
        registration_message_fun(client_id, message.content);

    if(message.type == MESSAGE_PING)
        ping_message_fun(client_id);

    if(message.type == MESSAGE_OUTCOME)
        result_message_fun(client_id, message.content);
}


int find_unused_id(){
    for(int i = 0; i < number_of_clients_limit; i++){
        if(clients_data[i].socket_fd == -1)
            return i;
    }
    return -1;
}


void connect_local(){
    //stworzenie struktury na adres gniazda klienta
    struct sockaddr_un socket_addr;
    
    //obliczenie wielkosci adresu
    socklen_t socket_addr_len = sizeof(struct sockaddr_un);

    //zaakceptowanie polaczenia
    int client_socket_fd = accept(socket_descriptor_local, (struct sockaddr *) &socket_addr, &socket_addr_len);
    if(client_socket_fd == -1){
        perror("Blad przy akceptowaniu polaczenia lokalnego");
        exit(1);
    }

    pthread_mutex_lock(&mutex);

    //znalezienie nieuzywanego id klienta
    int client_id = find_unused_id();

    if( client_id != -1){
        //wyslanie wiadomosci potwierdzajacej
        send_message(MESSAGE_ACCEPT, NULL, client_socket_fd);

        //zaktualizowanie danych klientow
        clients_data[client_id].socket_fd = client_socket_fd;
        printf("Zaakceptowano polaczenie lokalne\n");
    }
    else{
        //wyslanie wiadomosci o odrzuceniu
        send_message(MESSAGE_REFUSE, NULL, client_socket_fd);
        printf("Odrzucono polaczenie lokalne\n");
    }

    pthread_mutex_unlock(&mutex);
}


void connect_inet(){
    //stworzenie struktury na adres gniazda klienta
    struct sockaddr_in socket_addr;
    
    //obliczenie wielkosci adresu
    socklen_t socket_addr_len = sizeof(struct sockaddr_in);

    //zaakceptowanie polaczenia
    int client_socket_fd = accept(socket_descriptor_inet, (struct sockaddr *) &socket_addr, &socket_addr_len);
    if(client_socket_fd == -1){
        perror("Blad przy akceptowaniu polaczenia lokalnego");
        exit(1);
    }

    pthread_mutex_lock(&mutex);

    //znalezienie nieuzywanego id klienta
    int client_id = find_unused_id();
    
    if( client_id != -1){
        //wyslanie wiadomosci potwierdzajacej
        send_message(MESSAGE_ACCEPT, NULL, client_socket_fd);

        //zaktualizowanie danych klientow
        clients_data[client_id].socket_fd = client_socket_fd;
        printf("Zaakceptowano polaczenie sieciowe (%s:%d)\n", inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
    }
    else{
        //wyslanie wiadomosci o odrzuceniu
        send_message(MESSAGE_REFUSE, NULL, client_socket_fd);
        printf("Odrzucono polaczenie sieciowe (%s:%d)\n", inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
    }

    pthread_mutex_unlock(&mutex);
}


void delete_clients_data(int client_id){

    pthread_mutex_lock(&mutex);

    clients_data[client_id].socket_fd = -1;
    clients_data[client_id].registered = 0;
    clients_data[client_id].ponged = 0;

    pthread_mutex_unlock(&mutex);

}


void disconnect_client(int client_id){

    //zamkniecie kanalow komunikacji gniazda klienta
    shutdown(clients_data[client_id].socket_fd, SHUT_RDWR);

    //zamkniecie gniazda klienta
    close(clients_data[client_id].socket_fd);

    //usuniecie danych klienta
    delete_clients_data(client_id);
}


void * thread_connections(void * arg){

    //stworzenie tablicy akcji gniazd
    struct pollfd sockets_events[2 + number_of_clients_limit];
    
    //wypelnienie deskryptoru gniazda lokalnego serwera
    sockets_events[0].fd = socket_descriptor_local;

    //wypelnienie deskryptoru gniazda sieciowego serwera
    sockets_events[1].fd = socket_descriptor_inet;

    //wypelnienie akcji dla kazdego deskryptora - oczekiwanie na dane do odczytania
    for(int i = 0; i < number_of_clients_limit + 2; i++){
        sockets_events[i].events = POLLIN;
    }


    while(1){

        pthread_mutex_lock(&mutex);

        //zaktualizowanie deskryptorow
        for(int i = 0; i < number_of_clients_limit; i++){
            sockets_events[i+2].fd = clients_data[i].socket_fd;
            sockets_events[i+2].revents = 0;
        }

        pthread_mutex_unlock(&mutex);

        //oczekiwanie na zdarzenia na gniazdach
        if(poll(sockets_events, number_of_clients_limit + 2, -1) == -1){
            perror("Blad przy oczekiwaniu na dane do odczytania");
            exit(1);
        }

        //sprawdzenie lokalnego gniazda serwera
        if(sockets_events[0].revents & POLLIN && close_flag == 0){
            connect_local();
        }

        //sprawdzenie sieciowego gniazda serwera
        if(sockets_events[1].revents & POLLIN && close_flag == 0){
            connect_inet();
        }

        //obsluzenie akcji w gniazdach klientow
        for(int i = 2; i < number_of_clients_limit + 2; i++){

            //odlaczenia
            if(sockets_events[i].revents & POLLHUP){
                printf("Klient zostal zamkniety - odlaczanie %d - %s\n", i-2, clients_data[i-2].name);
                disconnect_client(i-2);
                continue;
            }
        
            //wiadomosci
            if(sockets_events[i].revents & POLLIN){
                receive_message(i-2);
            }
        }
    }
}

int get_next_client_id(){
    for(int i = 1; i <= number_of_clients_limit; i++){
        int id = (last_used_client + i) % number_of_clients_limit;
        if(clients_data[id].registered == 1){
            last_used_client = id;
            return id;
        }
    }
    return -1;
}

void * thread_pings(void * arg){
    while(1){
        //odczekanie sekundy
        sleep(2);

        //wybranie kolejnego klienta
        int client_id = get_next_client_id();
        if(client_id == -1){
            continue;
        }

        clients_data[client_id].ponged = 0;

        //wyslanie wiadomosci
        send_message(MESSAGE_PING, NULL, clients_data[client_id].socket_fd);
        printf("Wyslanie  wiadomosc PING do %d - %s\n", client_id, clients_data[client_id].name);

        //odczekanie sekundy
        sleep(1);

        //sprawdzenie czy klient odpowiedzial
        if(clients_data[client_id].ponged == 0){

            printf("Klient %d - %s nie odpowiedzial na PING, ", client_id, clients_data[client_id].name);
            if(clients_data[client_id].socket_fd == -1)
                printf("jest juz odlaczony\n");
            else{
                printf("odlaczanie\n");
                disconnect_client(client_id);
            }
        }
        
        if(clients_data[client_id].socket_fd != -1)
            printf("Odebranie wiadomosc PONG od %d - %s\n", client_id, clients_data[client_id].name);
    }
}


void sigint_fun(int signal_number){
    printf("Otrzymano sygnal SIGINT, zamykanie\n");
    exit(0);
}


void exit_fun(){

    close_flag = 1;

    //odlaczenie wszystkich klientow
    for(int i = 0; i < number_of_clients_limit; i++){
        disconnect_client(i);
    }

    //zamkniecie kanalow komunikacji gniazda lokalnego
    if(shutdown(socket_descriptor_local, SHUT_RDWR) != 0){
        perror("Blad przy zamykaniu kanalow komunikacji gniazda lokalnego");
    }

    //zamkniecie gniazda lokalnego
    if(close(socket_descriptor_local) != 0){
        perror("Blad przy zamykaniu gniazda lokalnego");
    }

    //usuniecie pliku reprezentujacego gniazdo lokalne
    if(unlink(unix_socket_path) != 0){
        perror("Blad przy usuwaniu pliku reprezentujacego gniazdo lokalne");
    }


    //zamkniecie kanalow komunikacji gniazda sieciowego
    if(shutdown(socket_descriptor_inet, SHUT_RDWR) != 0){
        perror("Blad przy zamykaniu kanalow komunikacji gniazda sieciowego");
    }

    //zamkniecie gniazda sieciowego
    if(close(socket_descriptor_inet) != 0){
        perror("Blad przy zamykaniu gniazda sieciowego");
    }

}


int main(int argc, char ** argv){

    //sprawdzenie ilosci argumentow wejsciowych
    if(argc != 3){
        perror("Zla liczba argumentow, uzycie:\n./server numer-portu sciezka-gniazda-UNIX");
        exit(1);
    }

    //przypisanie odpowiednich argumentow do zmiennych
    port_number = atoi(argv[1]);
    unix_socket_path = argv[2];


    //ustawienie funkcji wyjscia
    if(atexit(exit_fun) != 0){
        perror("Blad przy ustawianiu funckji wyjscia");
        exit(1);
    }


    //ustawienie obslugi sygnalu SIGINT
    struct sigaction sigact;
    sigact.sa_handler = sigint_fun;
    sigemptyset(&sigact.sa_mask);
    if(sigaction(SIGINT, &sigact, NULL) != 0){
        perror("Blad przy ustawieniu funkcji obslugi sygnalu SIGINT");
        exit(1);
    }


    //stworzenie gniazda lokalnego
    socket_descriptor_local = socket(AF_UNIX, SOCK_STREAM, 0);
    if(socket_descriptor_local == -1){
        perror("Blad przy towrzeniu gniazda lokalnego");
        exit(1);
    }

    //stworzenie gniazda sieciowego
    socket_descriptor_inet = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_descriptor_inet == -1){
        perror("Blad przy tworzeniu gniazda sieciowego");
        exit(1);
    }


    //stworzenie adresu gniazda lokalnego
    struct sockaddr_un socket_address_local;
    socket_address_local.sun_family = AF_UNIX;
    strcpy(socket_address_local.sun_path, unix_socket_path);

    //stworzenie adresu gniazda sieciowego
    struct sockaddr_in socket_address_inet;
    socket_address_inet.sin_family = AF_INET;
    socket_address_inet.sin_port = htons(port_number);
    socket_address_inet.sin_addr.s_addr = INADDR_ANY;


    //zbindowanie adresu gniazda lokalnego
    if(bind(socket_descriptor_local, (struct sockaddr *) &socket_address_local, sizeof(struct sockaddr_un)) != 0){
        perror("Blad przy bindowaniu adresu gniazda lokalnego");
        exit(1);
    }

    //zbindowanie adresu gniazda sieciowego
    if(bind(socket_descriptor_inet, (struct sockaddr *) &socket_address_inet, sizeof(struct sockaddr_in)) != 0){
        perror("Blad przy bindowaniu adresu gniazda sieciowego");
        exit(1);
    }


    //rozpoczecie nasluchiwania na polaczenia od klientow (gniazdo lokalne)
    if(listen(socket_descriptor_local, 15) != 0){
        perror("Blad przy rozpoczeciu nasluchiwania dla gniazda lokalnego");
        exit(1);
    }

    //rozpoczecie nasluchiwania na polaczenia od klientow (gniazdo lokalne)
    if(listen(socket_descriptor_inet, 15) != 0){
        perror("Blad przy rozpoczeciu nasluchiwania dla gniazda sieciowego");
        exit(1);
    }

    //zainicjalizowanie tablicy klientow
    for(int i = 0; i < number_of_clients_limit; i++){
        delete_clients_data(i);   
    }

    //wypisanie komunikatow o oczekiwaniu na polaczenia
    printf("Nasluchiwanie na polaczenia lokalne, sciezka: %s\n", unix_socket_path);
    printf("Nasluchiwanie na poleczenia sieciowe, port: %d\n", port_number);


    pthread_t thread_id;
    //utworzenie watku odpowiedzialnego za laczenie z klientami
    if(pthread_create(&thread_id, NULL, thread_connections, NULL) != 0){
        perror("Blad przy tworzeniu watku obslugujacego laczenie z klientami");
        exit(1);
    }

    //utworzenie watku odpowiedzialnego za pingowanie klientow
    if(pthread_create(&thread_id, NULL, thread_pings, NULL) != 0){
        perror("Blad przy tworzeniu watku pingujacego klientow");
        exit(1);
    }

    //obsluga wpisywaych dzialan
    while(1){

        //zwiekszenie licznika dzialan
        calculation_counter++;

        char calculation[300];
        char operation;
        int a, b;

        scanf("%s %d %d", &operation, &a, &b);

        if(operation != '+' && operation != '-' && operation != '*' && operation != '/'){
            printf("Bledna operacja\n");
            continue;
        }

        if(operation == '/' && b == 0){
            printf("Nie tym razem <3\n");
            continue;
        }


        sprintf(calculation, "%d\n%c %d %d", calculation_counter, operation, a, b);

        //znalezienie kolejnego klienta
        int client_id = get_next_client_id();
        if(client_id == -1){
            printf("Zaden klient nie moze obsluzyc dzialania\n");
            continue;
        }

        //wyslanie wiadomosci z zadaniem
        send_message(MESSAGE_CALCULATE, calculation, clients_data[client_id].socket_fd);
        printf("Wyslano dzialanie nr %s do klienta %d - %s\n", calculation, client_id, clients_data[client_id].name);
    }
}