// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// scheduler component for data archival.
// Called on each scheduler request.

#include <map>
#include <string>

#include "sched_types.h"

using std::map;
using std::string;

void handle_vda() {
    map<string, DB_VDA_CHUNK_HOST> chunks;
        // chunks that are supposed to be on this host

    DB_VDA_CHUNK_HOST ch;
    char buf[256];
    sprintf(buf, "host_id=%d", g_reply->host.id);
    while (1) {
        retval = ch.enumerate(buf);
        if (retval == ERR_DB_NOT_FOUND) break;
        if (retval) {
            // give up if we didn't get a complete enumeration
            //
            log_messages.printf(MSG_NORMAL,
                "[vda]: ch.enumerate() failed %d\n", retval
            );
            return;
        }
        chunks.insert(ch.name, ch);
    }

}
