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

// interfaces for accessing message queues

#include <stdio.h>
#include <string.h>

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

