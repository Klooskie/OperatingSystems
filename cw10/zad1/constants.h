#ifndef CONSTANTS_H
#define CONSTANTS_H

#define number_of_clients_limit 15

#define MESSAGE_ACCEPT 0
#define MESSAGE_REFUSE 1
#define MESSAGE_REGISTER 2
#define MESSAGE_PING 3
#define MESSAGE_CALCULATE 4
#define MESSAGE_OUTCOME 5

typedef struct msg msg;
struct msg{
    int type;
    char content[300];
};

void send_message(int message_type, char * message_content, int fd);
int get_message(msg * message, int fd);

#endif
