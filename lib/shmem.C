#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "shmem.h"

int create_shmem(key_t key, int size, void** pp){
    int id;
    id = shmget(key, size, IPC_CREAT|0777);
    if (id < 0) {
        perror("create_shmem: shmget");
        return -1;
    }
    return attach_shmem(key, pp);

}

int destroy_shmem(key_t key){
    struct shmid_ds buf;
    int id, retval;

    id = shmget(key, 0, 0);
    if (id < 0) return 0;           // assume it doesn't exist
    retval = shmctl(id, IPC_STAT, &buf);
    if (retval) return -1;
    if (buf.shm_nattch > 0) {
        fprintf(stderr,
            "destroy_shmem: can't destroy segment; %d attachments\n",
            buf.shm_nattch
        );
        return -1;
    }
    retval = shmctl(id, IPC_RMID, 0);
    if (retval) {
        fprintf(stderr, "destroy_shmem: remove failed %d\n", retval);
        return -1;
    }
    return 0;
}

int attach_shmem(key_t key, void** pp){
    void* p;
    int id;

    id = shmget(key, 0, 0);
    if (id < 0) {
        perror("attach_shmem: shmget");
        return -1;
    }
    p = shmat(id, 0, 0);
    if ((int)p == -1) {
        perror("attach_shmem: shmat");
        return -1;
    }
    *pp = p;
    return 0;
}

int detach_shmem(void* p) {
    int retval;

    retval = shmdt((char *)p);
    if (retval) perror("detach_shmem: shmdt");
    return retval;
}
