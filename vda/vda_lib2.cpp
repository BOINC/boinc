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

// code used in vda and vdad, but not in the simulator

#include "error_numbers.h"
#include "sched_msgs.h"
#include "util.h"

#include "vda_lib.h"

CHUNK::CHUNK(META_CHUNK* mc, double s, int index) {
    parent = mc;
    present_on_server = true;
    size = s;
    sprintf(name, "%s.%d", parent->name, index);
}

// assign this chunk to a host
//
int CHUNK::assign() {
    int host_id = parent->dfile->choose_host();
    if (!host_id) {
        return ERR_NOT_FOUND;
    }
    DB_VDA_CHUNK_HOST ch;
    ch.create_time = dtime();
    ch.vda_file_id = parent->dfile->id;
    strcpy(ch.name, name);
    ch.host_id = host_id;
    ch.present_on_host = 0;
    ch.transfer_in_progress = true;
    ch.transfer_wait = true;
    ch.transfer_request_time = ch.create_time;
    ch.transfer_send_time = 0;
    int retval = ch.insert();
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "ch.insert() failed\n");
        return retval;
    }
    return 0;
}

bool CHUNK::download_in_progress() {
    return false;
}

void CHUNK::start_upload() {
}

inline bool alive(DB_HOST& h) {
    return (h.rpc_time > dtime()-VDA_HOST_TIMEOUT);
}

char* host_alive_clause() {
    static char buf[256];
    sprintf(buf, "rpc_time > %f", dtime() - VDA_HOST_TIMEOUT);
    return buf;
}

// Pick a host to send a chunk of this file to.
// The host must:
// 1) be alive (recent RPC time)
// 2) not have any chunks of this file
//
// We maintain a cache of such hosts
// The policy is:
//
// - scan the cache, removing hosts that are no longer alive;
//   return if find a live host
// - pick a random starting point in host ID space,
//   and enumerate 100 live hosts; wrap around if needed.
//   Return one and put the rest in cache
//
int VDA_FILE_AUX::choose_host() {
    int retval;
    DB_HOST host;

    // replenish cache if needed
    //
    if (!available_hosts.size()) {
        int nhosts_scanned = 0;
        int rand_id;
        for (int i=0; i<2; i++) {
            char buf[256];
            if (i == 0) {
                retval = host.max_id(rand_id, "");
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "host.max_id() failed\n");
                    return 0;
                }
                rand_id = (int)(((double)id)*drand());
                sprintf(buf,
                    "where %s and id>=%d order by id limit 100",
                    host_alive_clause(), rand_id
                );
            } else {
                sprintf(buf,
                    "where %s and id<%d order by id limit %d",
                    host_alive_clause(), rand_id, 100-nhosts_scanned
                );
            }
            while (1) {
                retval = host.enumerate(buf);
                if (retval == ERR_DB_NOT_FOUND) break;
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "host enum failed\n");
                    return 0;
                }
                nhosts_scanned++;
                DB_VDA_CHUNK_HOST ch;
                char buf2[256];
                int count;
                sprintf(buf2, "where vda_file_id=%d and host_id=%d", id, host.id);
                retval = ch.count(count, buf2);
                if (retval) {
                    log_messages.printf(MSG_CRITICAL, "ch.count failed\n");
                    return 0;
                }
                if (count == 0) {
                    available_hosts.push_back(host.id);
                }
                if (nhosts_scanned == 100) break;
            }
            if (nhosts_scanned == 100) break;
        }
    }

    while (available_hosts.size()) {
        int hostid = available_hosts.back();
        available_hosts.pop_back();
        retval = host.lookup_id(hostid);
        if (retval || !alive(host)) {
            continue;
        }
        return hostid;
    }

    log_messages.printf(MSG_CRITICAL, "No hosts available\n");
    return 0;
}
