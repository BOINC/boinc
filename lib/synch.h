#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>

extern int create_semaphore(key_t);
extern int destroy_semaphore(key_t);
extern int lock_semaphore(key_t);
extern int unlock_semaphore(key_t);
extern int get_key(char* path, int id, key_t&);
