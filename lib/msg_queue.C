// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// interfaces for accessing message queues

#include <cstdio>
#include <cstring>

#include "msg_queue.h"

int create_message_queue(key_t key) {
    int mq_id;

    mq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (mq_id < 0) {
        perror("create_message_queue: msgget");
        return -1;
    }
    return 0;
}

int receive_message(key_t key, void *msg, size_t msg_size, bool wait) {
    int mq_id, retval;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        perror("receive_message: msgget");
        return -1;
    }

    retval = msgrcv(mq_id, msg, msg_size, 0, (wait?0:IPC_NOWAIT));
    if (retval < 0) {
        perror("receive_message: msgrcv");
        return -1;
    }

    return 0;
}

int send_message(key_t key, void *msg, size_t msg_size, bool wait) {
    int mq_id, retval;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        perror("send_message: msgget");
        return -1;
    }

    retval = msgsnd(mq_id, msg, msg_size, (wait?0:IPC_NOWAIT));
    if (retval < 0) {
        perror("send_message: msgsnd");
        return -1;
    }

    return 0;
}

int destroy_message_queue(key_t key) {
    int mq_id, retval;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        perror("delete_message_queue: msgget");
        return -1;
    }
    retval = msgctl(mq_id, IPC_RMID, NULL);
    if (retval) {
        perror("delete_message_queue: msgctl");
        return -1;
    }
    return 0;
}


const char *BOINC_RCSID_7b5e8a534b = "$Id$";
