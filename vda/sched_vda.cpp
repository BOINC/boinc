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

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_types.h"

using std::map;
using std::string;
using std::pair;

static int mark_for_update(int vda_file_id) {
    DB_VDA_FILE f;
    f.id = vda_file_id;
    return f.update_field("need_update=1");
}

// handle a scheduler request:
// - handle completed uploads
// - handle set of files present on client
//   (update or create VDA_CHUNK_HOST record)
// - handle files expected but not present
// - issue delete commands if needed to enforce share
// - issue upload or download commands to client
//
// relevant fields of SCHEDULER_REQUEST
// file_infos: list of sticky files
// file_xfer_results: list of completed file xfers
//
void handle_vda() {
    int retval;
    unsigned int i;
    map<string, DB_VDA_CHUNK_HOST> chunks;
        // chunks that are supposed to be on this host

    // get a list of the vda_chunk_host records for this host
    //
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
        if (config.debug_vda) {
            log_messages.printf(MSG_NORMAL,
                "[vda] DB: has chunk %s\n", ch.name
            );
        }
        chunks.insert(pair<string, DB_VDA_CHUNK_HOST>(string(ch.name), ch));
    }

    // process completed uploads:
    // - verify md5 of upload
    // - move it from upload dir to vda_file dir
    // - mark vda_file for update
    // - clear transfer_in_progress flag in vda_chunk_host
    //
    for (i=0; i<g_request->file_xfer_results.size(); i++) {
        RESULT& r = g_request->file_xfer_results[i];
        if (!starts_with(r.name, "upload_")) continue;
        //char* file_name = r.name + strlen("upload_");
        if (!strstr(r.name, "vdafile")) continue;
        if (config.debug_vda) {
            log_messages.printf(MSG_NORMAL,
                "[vda] DB: completed upload %s\n", r.name
            );
        }
    }

    // process list of present files;
    // - create a vda_chunk_host record if needed
    // - set present_on_host flag in vda_chunk_host
    // - mark our in-memory vda_chunk_host record as "found"
    // - mark vda_file for update
    //
    for (i=0; i<g_request->file_infos.size(); i++) {
        FILE_INFO& fi = g_request->file_infos[i];
        if (config.debug_vda) {
            log_messages.printf(MSG_NORMAL,
                "[vda] request: client has file %s\n", fi.name
            );
        }
    }

    // for each vda_chunk_host not in file list:
    // - delete from DB
    // - mark vda_file for update
    //
    map<string, DB_VDA_CHUNK_HOST>::iterator it;
    it = chunks.begin();
    while (it != chunks.end()) {
        DB_VDA_CHUNK_HOST& ch = (*it).second;
        if (!ch.found) {
            if (config.debug_vda) {
                log_messages.printf(MSG_NORMAL,
                    "[vda] in DB but not on client: %s\n", ch.name
                );
            }
            ch.delete_from_db();
            chunks.erase(it);
            mark_for_update(ch.vda_file_id);
        } else {
            it++;
        }
    }

    // if process is using more than its share of disk space,
    // remove some chunks and mark vda_files for update
    //
    if (g_request->host.d_project_share) {
        double x = g_request->host.d_boinc_used_project;
        if (config.debug_vda) {
            log_messages.printf(MSG_NORMAL,
                "[vda] share: %f used: %f\n",
                g_request->host.d_project_share, x
            );
        }
        it = chunks.begin();
        while (x > g_request->host.d_project_share && it != chunks.end()) {
            DB_VDA_CHUNK_HOST& ch = (*it).second;
            FILE_INFO fi;
            strcpy(fi.name, ch.name);
            if (config.debug_vda) {
                log_messages.printf(MSG_NORMAL,
                    "[vda] deleting: %s\n", ch.name
                );
            }
            //x -= ch.size;
            g_reply->file_deletes.push_back(fi);
            it++;
        }
    }

    // issue upload and download commands
    //
    it = chunks.begin();
    while (it != chunks.end()) {
        DB_VDA_CHUNK_HOST& ch = (*it).second;
        if (!ch.transfer_in_progress) continue;
        if (!ch.transfer_wait) continue;
        if (ch.present_on_host) {
            if (config.debug_vda) {
                log_messages.printf(MSG_NORMAL,
                    "[vda] sending upload command: %s\n", ch.name
                );
            }
            // upload
        } else {
            if (config.debug_vda) {
                log_messages.printf(MSG_NORMAL,
                    "[vda] sending download command: %s\n", ch.name
                );
            }
            // download
        }
    }
}
