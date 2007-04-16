#include <sys/sem.h>

extern int create_semaphore(key_t);
extern int destroy_semaphore(key_t);
extern int lock_semaphore(key_t);
extern int unlock_semaphore(key_t);
