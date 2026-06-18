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

// logic for "asynchronous" file copy and unzip/verify operations.
// "asynchronous" means that the the operations are done in 64KB chunks
// in the client's polling loop,
// so that the client continues to respond to GUI RPCs
// and the manager won't freeze.

#ifdef _WIN32
#include "boinc_win.h"
#else
#include <string.h>
#endif

#include "crypt.h"
#include "error_numbers.h"
#include "filesys.h"
#include "md5_file.h"
#include "str_replace.h"

#include "app.h"
#include "client_msgs.h"
#include "client_state.h"
#include "project.h"
#include "sandbox.h"

#include "async_file.h"

using std::vector;

vector<ASYNC_VERIFY*> async_verifies;
vector<ASYNC_COPY*> async_copies;

#define BUFSIZE (64*1024)

// set up an async copy operation.
//
int ASYNC_COPY::init(
    ACTIVE_TASK* _atp, FILE_INFO* _fip,
    const char* from_path, const char* _to_path
) {
    atp = _atp;
    fip = _fip;
    safe_strcpy(to_path, _to_path);

    if (log_flags.async_file_debug) {
        msg_printf(atp->wup->project, MSG_INFO,
            "[async] started async copy of %s", from_path
        );
    }
    in = boinc_fopen(from_path, "rb");
    if (!in) return ERR_FOPEN;

    // Arrange to copy the file to a temp file in the target directory.
    // It will be renamed later.
    // Use mkstemp() rather than tempnam() because the latter gives priority
    // to env var for temp directory; don't want this.
    //
    char dir[MAXPATHLEN];
    boinc_path_to_dir(to_path, dir);
#ifdef _WIN32
    out = boinc_temp_file(dir, "cpy", temp_path, fip->nbytes);
#else
    out = boinc_temp_file(dir, "copy", temp_path);
#endif
    if (!out) {
        fclose(in);
        return ERR_FOPEN;
    }
    atp->async_copy = this;
    async_copies.push_back(this);
    return 0;
}

ASYNC_COPY::ASYNC_COPY() {
    in = out = NULL;
    atp = NULL;
    fip = NULL;
    safe_strcpy(to_path, "");
    safe_strcpy(temp_path, "");
}

ASYNC_COPY::~ASYNC_COPY() {
    if (in) fclose(in);
    if (out) fclose(out);
    if (atp) {
        atp->async_copy = NULL;
    }
}

// copy a 64KB chunk.
// return nonzero if we're done (success or fail)
//
int ASYNC_COPY::copy_chunk() {
    unsigned char buf[BUFSIZE];
    int retval;

    size_t n = fread(buf, 1, BUFSIZE, in);
    if (n == 0) {
        // copy done.  rename temp file
        //
        fclose(in);
        fclose(out);
        in = out = NULL;
        retval = boinc_rename(temp_path, to_path);
        if (retval) {
            error(retval);
            return 1;
        }

        if (log_flags.async_file_debug) {
            msg_printf(atp->wup->project, MSG_INFO,
                "[async] async copy of %s finished", to_path
            );
        }

        atp->async_copy = NULL;
        fip->set_permissions(to_path);

        // If task is still scheduled, start it.
        //
        if (atp->scheduler_state == CPU_SCHED_SCHEDULED) {
            retval = atp->start();
            if (retval) {
                error(retval);
            }
        }
        return 1;       // tell caller we're done
    } else {
        size_t m = fwrite(buf, 1, n, out);
        if (m != n) {
            error(ERR_FWRITE);
            return 1;
        }
    }
    return 0;
}

// handle the failure of a copy; error out the result
//
void ASYNC_COPY::error(int retval) {
    char err_msg[4096];
    atp->set_task_state(PROCESS_COULDNT_START, "ASYNC_COPY::error");
    snprintf(err_msg, sizeof(err_msg),
        "Couldn't copy file: %s", boincerror(retval)
    );
    gstate.report_result_error(*(atp->result), err_msg);
    gstate.request_schedule_cpus("start failed");
}

void remove_async_copy(ASYNC_COPY* acp) {
    vector<ASYNC_COPY*>::iterator i = async_copies.begin();
    while (i != async_copies.end()) {
        if (*i == acp) {
            async_copies.erase(i);
            break;
        }
        ++i;
    }
    delete acp;
}

int ASYNC_VERIFY::init(FILE_INFO* _fip) {
    fip = _fip;
    md5_init(&md5_state);
    get_pathname(fip, inpath, sizeof(inpath));

    if (log_flags.async_file_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[async] started async MD5%s of %s",
            fip->download_gzipped?" and uncompress":"", fip->name
        );
    }
    if (fip->download_gzipped) {
        safe_strcpy(outpath, inpath);
        char dir[MAXPATHLEN];
        boinc_path_to_dir(outpath, dir);
#ifdef _WIN32
        out = boinc_temp_file(dir, "vfy", temp_path, fip->nbytes);
#else
        out = boinc_temp_file(dir, "verify", temp_path);
#endif
        if (!out) {
            fclose(in);
            return ERR_FOPEN;
        }

        safe_strcat(inpath, ".gz");
        gzin = gzopen(inpath, "rb");
        if (gzin == Z_NULL) {
            fclose(out);
            boinc_delete_file(temp_path);
            return ERR_FOPEN;
        }
    } else {
        in = boinc_fopen(inpath, "rb");
        if (!in) return ERR_FOPEN;
    }
    async_verifies.push_back(this);
    fip->async_verify = this;
    return 0;
}

// the MD5 has been computed.  Finish up.
//
void ASYNC_VERIFY::finish() {
    unsigned char binout[16];
    char md5_buf[64];
    int retval;

    md5_finish(&md5_state, binout);
    for (int i=0; i<16; i++) {
        sprintf(md5_buf+2*i, "%02x", binout[i]);
    }
    md5_buf[32] = 0;
    if (fip->signature_required) {
        bool verified;
        retval = check_file_signature2(md5_buf, fip->file_signature,
            fip->project->code_sign_key, verified
        );
        if (retval) {
            error(retval);
            return;
        }
        if (!verified) {
            error(ERR_RSA_FAILED);
            return;
        }
    } else {
        if (strcmp(md5_buf, fip->md5_cksum)) {
            error(ERR_MD5_FAILED);
            return;
        }
    }
    if (log_flags.async_file_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[async] async verify of %s finished", fip->name
        );
    }
    fip->async_verify = NULL;
    fip->status = FILE_PRESENT;
    fip->set_permissions();
}

void ASYNC_VERIFY::error(int retval) {
    if (log_flags.async_file_debug) {
        msg_printf(fip->project, MSG_INFO,
            "[async] async verify of %s failed: %s",
            fip->name, boincerror(retval)
        );
    }
    fip->async_verify = NULL;
    fip->status = retval;
}

int ASYNC_VERIFY::verify_chunk() {
    size_t n;
    unsigned char buf[BUFSIZE];
    if (fip->download_gzipped) {
        n = gzread(gzin, buf, BUFSIZE);
        if (n <=0) {
            // done
            //
            gzclose(gzin);
            fclose(out);
            delete_project_owned_file(inpath, true);
            boinc_rename(temp_path, outpath);
            finish();
            return 1;
        } else {
            size_t m = fwrite(buf, 1, n, out);
            if (m != n || ferror(out)) {
                // write failed
                //
                error(ERR_FWRITE);
                return 1;
            }
            md5_append(&md5_state, buf, (int)n);
        }
    } else {
        n = fread(buf, 1, BUFSIZE, in);
        if (!n || ferror(in)) {
            fclose(in);
            finish();
            return 1;
        } else {
            md5_append(&md5_state, buf, (int)n);
        }
    }
    return 0;
}

void remove_async_verify(ASYNC_VERIFY* avp) {
    vector<ASYNC_VERIFY*>::iterator i = async_verifies.begin();
    while (i != async_verifies.end()) {
        if (*i == avp) {
            async_verifies.erase(i);
            break;
        }
        ++i;
    }
    delete avp;
}

// If there are any async file operations,
// do a 64KB chunk of the first one and return true.
//
// Note: if there are lots of pending operations,
// it's better to finish the oldest one before starting the rest
//
void do_async_file_op() {
    if (async_copies.size()) {
        ASYNC_COPY* acp = async_copies[0];
        if (acp->copy_chunk()) {
            async_copies.erase(async_copies.begin());
            delete acp;
        }
        return;
    }
    if (async_verifies.size()) {
        if (async_verifies[0]->verify_chunk()) {
            async_verifies.erase(async_verifies.begin());
        }
    }
}

