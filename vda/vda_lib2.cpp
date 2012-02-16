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
void CHUNK::assign() {
    int host_id = parent->dfile->choose_host();
    if (!host_id) return;
    DB_VDA_CHUNK_HOST ch;
    ch.host_id = host_id;
    strcpy(ch.name, name);
    int retval = ch.insert();
    if (retval) {
        fprintf(stderr, "ch.insert() failed\n");
    }
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
// Let N(H) be the number of chunks of this file on host H.
// We maintain a cache of hosts with N(H)=0.
// The policy is:
//
// - scan the cache, removing hosts that are no longer alive;
//   return if find a live host
// - pick a random starting point in host ID space,
//   and enumerate 100 live hosts; wrap around if needed.
//   If find any with N(H)=0, return one and put the rest in cache
// - else return the one for which N(H) is least
//
int VDA_FILE_AUX::choose_host() {
    DB_HOST host;
    int retval;
    while (available_hosts.size()) {
        int hostid = available_hosts.back();
        available_hosts.pop_back();
        retval = host.lookup_id(hostid);
        if (retval || !alive(host)) {
            continue;
        }
        return hostid;
    }

    int host0_id = 0;   // ID of host with N(H)=0, if any
    int best_host_id = 0;
    int best_host_n = 0;
    int nhosts_scanned = 0;
    for (int i=0; i<2; i++) {
        char buf[256];
        int rand_id;
        if (i == 0) {
            retval = host.max_id(rand_id, "");
            if (retval) {
                fprintf(stderr, "host.max_id() failed\n");
                return 0;
            }
            rand_id = (int)(((double)id)*drand());
            sprintf(buf,
                "where %s and id>=%d limit 100 order by id",
                host_alive_clause(), rand_id
            );
        } else {
            sprintf(buf,
                "where %s and id<%d limit %d order by id",
                host_alive_clause, rand_id, 100-nhosts_scanned
            );
        }
        while (1) {
            int retval = host.enumerate(buf);
            if (retval == ERR_DB_NOT_FOUND) break;
            if (retval) {
                fprintf(stderr, "host enum failed\n");
                return 0;
            }
            nhosts_scanned++;
            DB_VDA_CHUNK_HOST ch;
            char buf2[256];
            int count;
            sprintf(buf2, "vda_file_id=%d and host_id=%d", id, host.id);
            retval = ch.count(count, buf2);
            if (retval) {
                fprintf(stderr, "ch.count failed\n");
                return 0;
            }
            if (count == 0) {
                if (host0_id) {
                    available_hosts.push_back(host.id);
                } else {
                    host0_id = host.id;
                }
            } else {
                if (best_host_id) {
                    if (count < best_host_n) {
                        best_host_n = count;
                        best_host_id = host.id;
                    }
                } else {
                    best_host_n = count;
                    best_host_id = host.id;
                }
            }
        }
        if (nhosts_scanned == 100) break;
    }
    if (host0_id) return host0_id;
    if (best_host_id) return best_host_id;
    fprintf(stderr, "No hosts available\n");
    return 0;
}
