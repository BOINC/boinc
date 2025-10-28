// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// interfaces for accessing message queues

#include "config.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "boinc_stdio.h"
#include "msg_queue.h"

int create_message_queue(key_t key) {
    int mq_id;

    mq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if (mq_id < 0) {
        boinc::perror("create_message_queue: msgget");
        return -1;
    }
    return 0;
}

int receive_message(key_t key, void *msg, size_t msg_size, bool wait) {
    int mq_id;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        boinc::perror("receive_message: msgget");
        return -1;
    }

    ssize_t retval = msgrcv(mq_id, msg, msg_size, 0, (wait?0:IPC_NOWAIT));
    if (retval < 0) {
        boinc::perror("receive_message: msgrcv");
        return -1;
    }

    return 0;
}

int send_message(key_t key, void *msg, size_t msg_size, bool wait) {
    int mq_id, retval;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        boinc::perror("send_message: msgget");
        return -1;
    }

    retval = msgsnd(mq_id, msg, msg_size, (wait?0:IPC_NOWAIT));
    if (retval < 0) {
        boinc::perror("send_message: msgsnd");
        return -1;
    }

    return 0;
}

int destroy_message_queue(key_t key) {
    int mq_id, retval;

    mq_id = msgget(key, 0666);
    if (mq_id < 0) {
        boinc::perror("delete_message_queue: msgget");
        return -1;
    }
    retval = msgctl(mq_id, IPC_RMID, NULL);
    if (retval) {
        boinc::perror("delete_message_queue: msgctl");
        return -1;
    }
    return 0;
}
