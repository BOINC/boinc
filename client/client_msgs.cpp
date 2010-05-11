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

#ifdef _WIN32
#include "boinc_win.h"
#else
#include "config.h"
#include <cstdarg>
#include <cstring>
#include <deque>
#endif
#include "str_util.h"

using std::deque;

#include "log_flags.h"
#include "str_replace.h"

#include "client_types.h"
#include "client_state.h"
#include "cs_notice.h"
#include "main.h"

#include "client_msgs.h"

MESSAGE_DESCS message_descs;

// Takes a printf style formatted string, inserts the proper values,
// and passes it to show_message
//
void msg_printf(PROJECT *p, int priority, const char *fmt, ...) {
    char buf[8192];  // output can be much longer than format
    va_list ap;

    if (fmt == NULL) return;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    buf[sizeof(buf)-1] = 0;
    va_end(ap);

    show_message(p, buf, priority);
}

// handle new message:
// add to cache, and delete old messages if cache too big.
// If high priority, create a notice.
//
void MESSAGE_DESCS::insert(PROJECT* p, int priority, int now, char* message) {
    MESSAGE_DESC* mdp = new MESSAGE_DESC;
    static int seqno = 1;
    strcpy(mdp->project_name, "");
    if (p) {
        strlcpy(
            mdp->project_name, p->get_project_name(), sizeof(mdp->project_name)
        );
    }
    mdp->priority = priority;
    mdp->timestamp = now;
    mdp->seqno = seqno++;
    mdp->message = message;
    while (msgs.size() > MAX_SAVED_MESSAGES) {
        delete msgs.back();
        msgs.pop_back();
    }
    msgs.push_front(mdp);

#ifndef SIM
    if (priority == MSG_USER_ALERT) {
        NOTICE n;
        n.description = message;
        if (p) {
            strcpy(n.project_name, p->get_project_name());
        }
        n.create_time = n.arrival_time = gstate.now;
        strcpy(n.category, "client");
        notices.append(n, false);
    }
#endif
}

void MESSAGE_DESCS::write(int seqno, MIOFILE& fout) {
    int i, j;
    unsigned int k;
    MESSAGE_DESC* mdp;

    // messages are stored in descreasing seqno,
    // i.e. newer ones are at the head of the vector.
    // compute j = index of first message to return
    //
    j = (int)msgs.size()-1;
    for (k=0; k<msgs.size(); k++) {
        mdp = msgs[k];
        if (mdp->seqno <= seqno) {
            j = k-1;
            break;
        }
    }

    fout.printf("<msgs>\n");
    for (i=j; i>=0; i--) {
        mdp = msgs[i];
        fout.printf(
            "<msg>\n"
            " <project>%s</project>\n"
            " <pri>%d</pri>\n"
            " <seqno>%d</seqno>\n"
            " <body>\n%s\n</body>\n"
            " <time>%d</time>\n",
            mdp->project_name,
            mdp->priority,
            mdp->seqno,
            mdp->message.c_str(),
            mdp->timestamp
        );
        fout.printf("</msg>\n");
    }
    fout.printf("</msgs>\n");
}

int MESSAGE_DESCS::highest_seqno() {
    if (msgs.size()) return msgs[0]->seqno;
    return 0;
}

void MESSAGE_DESCS::cleanup() {
    for (unsigned int i=0; i<msgs.size(); i++) {
        delete msgs[i];
    }
    msgs.clear();
}
