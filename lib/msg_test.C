// test program for message queue functions

// -d       destroy
// -c       create
// -s [msg] send message [msg]
// -r       receive message
// -rw      wait for message

#define KEY 0xb01fcafe

#include <string.h>
#include <stdio.h>

#include "msg_queue.h"

struct my_msg {
    long msg_type;
    char msg_text[256];
};

int main(int argc, char** argv) {
    my_msg the_msg;
    int retval;

    if (!strcmp(argv[1], "-d")) {
        destroy_message_queue(KEY);
    } else if (!strcmp(argv[1], "-c")) {
        create_message_queue(KEY);
    } else if (!strcmp(argv[1], "-rw")) {
        retval = receive_message(KEY, &the_msg, sizeof(my_msg),true);
        printf("Received message: %s\n", the_msg.msg_text);
    } else if (!strcmp(argv[1], "-r")) {
        retval = receive_message(KEY, &the_msg, sizeof(my_msg),false);
        if (!retval) printf("Received message: %s\n", the_msg.msg_text);
    } else if (!strcmp(argv[1], "-s")) {
        the_msg.msg_type = 1;
        strcpy(the_msg.msg_text, argv[2]);
        send_message(KEY, &the_msg, sizeof(my_msg),true);
    }

    return 0;
}
