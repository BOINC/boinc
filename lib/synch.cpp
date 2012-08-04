// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.


// interfaces for accessing sempahores

#include "synch.h"

#ifndef ANDROID //disabled on Android

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

using std::memset;

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "error_numbers.h"

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

#endif //ANDROID
