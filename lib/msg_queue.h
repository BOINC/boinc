#include <sys/msg.h>

extern int create_message_queue(key_t);
extern int receive_message(key_t,void*,size_t,bool);
extern int send_message(key_t,void*,size_t,bool);
extern int destroy_message_queue(key_t);
