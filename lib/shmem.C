// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//


// interfaces for accessing shared memory segments

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>

#include "shmem.h"

int create_shmem(key_t key, int size, void** pp){
    int id;
    assert(pp!=NULL);
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
    assert(pp!=NULL);
    //fprintf(stderr, "%x\n", key);
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
    assert(p!=NULL);
    retval = shmdt((char *)p);
    if (retval) perror("detach_shmem: shmdt");
    return retval;
}
