#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_MSG_H
#include <sys/msg.h>
#endif

extern int create_message_queue(key_t);
extern int receive_message(key_t,void*,size_t,bool);
extern int send_message(key_t,void*,size_t,bool);
extern int destroy_message_queue(key_t);
