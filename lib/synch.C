#include <stdio.h>
#include <string.h>

#include "synch.h"

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *arra;
    struct seminfo *__buf;
};

int create_semaphore(key_t key){
    int id, retval;
    semun s;

    id = semget(key, 1, IPC_CREAT|IPC_EXCL|0777);
    if (id < 0) {
        perror("create_semaphore: semget");
        return -1;
    }
    memset(&s, 0, sizeof(s));
    s.val = 1;
    retval = semctl(id, 0, SETVAL, s);
    if (retval) {
        perror("create_semaphore: semctl");
        return -1;
    }
    return 0;
}

int destroy_semaphore(key_t key){
    int id, retval;
    id = semget(key, 0, 0);
    if (id < 0) {
        perror("destroy_semaphore: semget");
        return -1;
    }
    retval = semctl(id, 1, IPC_RMID, 0);
    if (retval) {
        perror("destroy_semaphore: semctl");
        return -1;
    }
    return 0;
}

int lock_semaphore(key_t key) {
    struct sembuf s;
    int id, retval;

    id = semget(key, 0, 0);
    if (id < 0) {
        perror("lock_semaphore: semget");
        return -1;
    }
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = SEM_UNDO;
    retval = semop(id, &s, 1);
    if (retval) {
        perror("lock_semaphore: semctl");
        return -1;
    }
    return 0;
}

int unlock_semaphore(key_t key) {
    struct sembuf s;
    int id, retval;

    id = semget(key, 0, 0);
    if (id < 0) {
        perror("unlock_semaphore: semget");
        return -1;
    }
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = SEM_UNDO;
    retval = semop(id, &s, 1);
    if (retval) {
        perror("unlock_semaphore: semctl");
        return -1;
    }
    return 0;
}

