#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "constants.h"


void send_message(int message_type, char * message_content, int fd){

    msg message;
    message.type = message_type;
    if(message_content != NULL)
        strcpy(message.content, message_content);

    if(write(fd, &message, sizeof(msg)) == -1){
        perror("Blad przy wysylaniu wiadomosci");
        exit(1);
    }

}

int  get_message(msg* message, int fd){

    int msg_size = read(fd, message, sizeof(msg));
    if(msg_size == -1){
        perror("Blad przy odbieraniu wiadomosci");
        exit(1);
    }
    return msg_size;

}