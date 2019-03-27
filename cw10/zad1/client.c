#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "constants.h"


char * server_addr;
int port_number;

char * client_name;
int socket_type;

int socket_descriptor;


void ping_message_fun(){
    //wyslanie wiadomosci zwrotnej
    send_message(MESSAGE_PING, NULL, socket_descriptor);
    printf("Wysano wiadomosc PONG\n");
}

void calculate_message_fun(char * calculation){

    printf("Otrzymano dzialanie %s\n", calculation);

    char operation;
    int counter, a, b, result;

    sscanf(calculation, "%d %c %d %d", &counter, &operation, &a, &b);

    if(operation == '+')
        result = a + b;

    if(operation == '-')
        result = a - b;

    if(operation == '*')
        result = a * b;

    if(operation == '/'){
        result = a / b;
    }
    
    char answer[300];
    sprintf(answer, "(%d) %d %c %d = %d", counter, a, operation, b, result);

    send_message(MESSAGE_OUTCOME, answer, socket_descriptor);
    printf("Odeslano odpowiedz - %s\n", answer);

}


void sigint_fun(int signal_number){
    printf("Otrzymano sygnal SIGINT, zamykanie\n");
    exit(0);
}


void exit_fun(){

    //zamkniecie kanalow komunikacji gniazda klienta
    if(shutdown(socket_descriptor, SHUT_RDWR) != 0){
        perror("Blad przy zamykaniu kanalow komunikacji gniazda");
    }

    //zamkniecie gniazda klienta
    if(close(socket_descriptor) != 0){
        perror("Blad przy zamykaniu gniazda");
    }

}


void wrong_number_of_arguments(){
    printf("Zla liczba argumentow, uzycie:\n./client nazwa-klienta sposob-komunikacji(local/inet) adres-serwera(IPv4/sciezka-gniazda-UNIX) numer-portu(w-przypadku-IPv4)\n");
    exit(1);
}


int main(int argc, char ** argv){

    //sprawdzenie ilosci argumentow wejsciowych
    if(argc != 4 && argc != 5)
        wrong_number_of_arguments();

    //przypisanie odpowiednich argumentow do zmiennych
    if(strcmp(argv[2], "local") == 0){
        client_name = argv[1];
        socket_type = AF_UNIX;
        server_addr = argv[3];
        if(argc != 4)
            wrong_number_of_arguments();
    }
    else if(strcmp(argv[2], "inet") == 0){
        client_name = argv[1];
        socket_type = AF_INET;
        server_addr = argv[3];
        if(argc != 5)
            wrong_number_of_arguments();
        port_number = atoi(argv[4]);        
    }
    else{
        printf("Bledne argumenty wejsciowe (sposob komunikacji)\n");
        exit(1);
    }


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


    //ustalenie adresu gniazda serwera
    struct sockaddr * server_socket_addr;
    int server_socket_addr_len;

    if(socket_type == AF_UNIX){
        //stworzenie adresu gniazda lokalnego
        struct sockaddr_un socket_addr_local;
        socket_addr_local.sun_family = AF_UNIX;
        strcpy(socket_addr_local.sun_path, server_addr);

        server_socket_addr = (struct sockaddr *) &socket_addr_local;
        server_socket_addr_len = sizeof(struct sockaddr_un);
    }
    else{
        //stworzenie adresu gniazda sieciowego
        struct sockaddr_in socket_addr_inet;
        socket_addr_inet.sin_family = AF_INET;
        socket_addr_inet.sin_port = htons(port_number);
        if(inet_pton(AF_INET, server_addr, &socket_addr_inet.sin_addr) != 1){
            printf("Blad przy probie konwersji adresu IPv4 na odpowiednia strukture\n");
            exit(1);
        }

        server_socket_addr = (struct sockaddr *) &socket_addr_inet;
        server_socket_addr_len = sizeof(struct sockaddr_in);
    }


    //stworzenie gniazda klienta
    if((socket_descriptor = socket(socket_type, SOCK_STREAM, 0)) == -1){
        perror("Blad przy tworzeniu gniazda");
        exit(1);
    }

    //polaczenie z serwerem
    if(connect(socket_descriptor, server_socket_addr, server_socket_addr_len) != 0){
        perror("Blad przy laczeniu sie z serwerem");
        exit(1);
    }

    //oczekiwanie na odpowiedz
    msg message_connection;
    get_message(&message_connection, socket_descriptor);
    if(message_connection.type == MESSAGE_REFUSE){
        printf("Serwer odmowil na polaczenie \n");
        exit(0);
    }

    if(socket_type == AF_UNIX)
        printf("Polaczono do servera lokalnego (%s)", server_addr);
    else
        printf("Polaczono do servera sieciowego (%s:%d)\n", server_addr, port_number);

    //rejestracja
    send_message(MESSAGE_REGISTER, client_name, socket_descriptor);

    //oczekiwanie na odpowiedz
    msg message_registration;
    get_message(&message_registration, socket_descriptor);
    if(message_registration.type == MESSAGE_REFUSE){
        printf("Serwer odmowil na polaczenie \n");
        exit(1);
    }

    printf("Polaczono z serwerem\n");

    //odbieranie kolejnych wiadomosci
    while(1){
        msg message;
        if(get_message(&message, socket_descriptor) == 0){
            printf("Serwer zostal zakmniety, zamykanie klienta\n");
            exit(0);
        }

        if(message.type == MESSAGE_PING)
            ping_message_fun();
        
        if(message.type == MESSAGE_CALCULATE)
            calculate_message_fun(message.content);

    }

}