static volatile const char *BOINCrcsid="$Id$";
// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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


// interfaces for accessing sempahores

#include <cstdio>
#include <cstring>

#include "error_numbers.h"
#include "synch.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

union SEMUN {
    int val;
    struct semid_ds *buf;
    unsigned short int *arra;
    struct seminfo *__buf;
};

int create_semaphore(key_t key){
    int id, retval;
    SEMUN s;

    id = semget(key, 1, IPC_CREAT|IPC_EXCL|0777);
    if (id < 0) {
        return ERR_SEMGET;
    }
    memset(&s, 0, sizeof(s));
    s.val = 1;
    retval = semctl(id, 0, SETVAL, s);
    if (retval) {
        return ERR_SEMCTL;
    }
    return 0;
}

int destroy_semaphore(key_t key){
    int id, retval;
    id = semget(key, 0, 0);
    if (id < 0) {
        return ERR_SEMGET;
    }
    retval = semctl(id, 1, IPC_RMID, 0);
    if (retval) {
        return ERR_SEMCTL;
    }
    return 0;
}

int lock_semaphore(key_t key) {
    struct sembuf s;
    int id, retval;

    id = semget(key, 0, 0);
    if (id < 0) {
        return ERR_SEMGET;
    }
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = SEM_UNDO;
    retval = semop(id, &s, 1);
    if (retval) {
        return ERR_SEMOP;
    }
    return 0;
}

int unlock_semaphore(key_t key) {
    struct sembuf s;
    int id, retval;

    id = semget(key, 0, 0);
    if (id < 0) {
        return ERR_SEMGET;
    }
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = SEM_UNDO;
    retval = semop(id, &s, 1);
    if (retval) {
        return ERR_SEMOP;
    }
    return 0;
}

int get_key(char* path, int id, key_t& key) {
    key = ftok(path, id);
    if (key == (key_t)-1) return ERR_FTOK;
    return 0;
}
