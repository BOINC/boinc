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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#include "zlib.h"
#else
#include "config.h"
// Somehow having config.h define _FILE_OFFSET_BITS or _LARGE_FILES is
// causing open to be redefined to open64 which somehow, in some versions
// of zlib.h causes gzopen to be redefined as gzopen64 which subsequently gets
// reported as a linker error.  So for this file, we compile in small files
// mode, regardless of these settings
#undef _FILE_OFFSET_BITS
#undef _LARGE_FILES
#undef _LARGEFILE_SOURCE
#undef _LARGEFILE64_SOURCE
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <cstring>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "log_flags.h"
#include "md5.h"
#include "parse.h"
#include "str_util.h"
#include "str_replace.h"
#include "util.h"

#include "async_file.h"
#include "client_msgs.h"
#include "client_state.h"
#include "file_names.h"
#include "project.h"
#include "pers_file_xfer.h"
#include "project.h"
#include "sandbox.h"

#include "client_types.h"

using std::string;
using std::vector;

bool FILE_XFER_BACKOFF::ok_to_transfer() {
    double dt = next_xfer_time - gstate.now;
    if (dt > gstate.pers_retry_delay_max) {
        // must have changed the system clock
        //
        dt = 0;
    }
    return (dt <= 0);
}

// A transfer has failed.
// Back off transfers (project-wide) if needed.
//
void FILE_XFER_BACKOFF::file_xfer_failed(PROJECT* p) {
    // If we're already backed off, ignore this failure.
    // If we start several transfers at once
    // (say, N output files of a job) and they all fail,
    // we don't want to back off N times, which could be hours.
    //
    if (gstate.now < next_xfer_time) {
        return;
    }

    file_xfer_failures++;
    if (file_xfer_failures < FILE_XFER_FAILURE_LIMIT) {
        next_xfer_time = 0;
        return;
    }
    double backoff = calculate_exponential_backoff(
        file_xfer_failures,
        gstate.pers_retry_delay_min,
        gstate.pers_retry_delay_max
    );
    if (log_flags.file_xfer_debug) {
        msg_printf(p, MSG_INFO,
            "[file_xfer] project-wide %s delay for %f sec",
            is_upload?"upload":"download",
            backoff
        );
    }
    next_xfer_time = gstate.now + backoff;
}

void FILE_XFER_BACKOFF::file_xfer_succeeded() {
    file_xfer_failures = 0;
    next_xfer_time  = 0;
}

int parse_project_files(XML_PARSER& xp, vector<FILE_REF>& project_files) {
    int retval;
    project_files.clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/project_files")) return 0;
        if (xp.match_tag("file_ref")) {
            FILE_REF file_ref;
            retval = file_ref.parse(xp);
            if (retval) {
                msg_printf(0, MSG_INFO,
                    "can't parse file_ref in project file: %s",
                    boincerror(retval)
                );
            } else {
                project_files.push_back(file_ref);
            }
        } else {
            if (log_flags.unparsed_xml) {
                msg_printf(0, MSG_INFO,
                    "[unparsed_xml] parse_project_files(): unrecognized: %s\n",
                    xp.parsed_tag
                );
            }
            xp.skip_unexpected();
        }
    }
    return ERR_XML_PARSE;
}

int APP::parse(XML_PARSER& xp) {
    safe_strcpy(name, "");
    safe_strcpy(user_friendly_name, "");
    project = NULL;
    non_cpu_intensive = false;
    sporadic = false;
    while (!xp.get_tag()) {
        if (xp.match_tag("/app")) {
            if (!strlen(user_friendly_name)) {
                safe_strcpy(user_friendly_name, name);
            }
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("user_friendly_name", user_friendly_name, sizeof(user_friendly_name))) continue;
        if (xp.parse_bool("non_cpu_intensive", non_cpu_intensive)) continue;
        if (xp.parse_bool("sporadic", sporadic)) continue;
        if (xp.parse_bool("fraction_done_exact", fraction_done_exact)) continue;
        if (xp.parse_bool("sporadic", sporadic)) {
            if (sporadic) gstate.have_sporadic_app = true;
            continue;
        }
#ifdef SIM
        if (xp.parse_double("latency_bound", latency_bound)) continue;
        if (xp.parse_double("fpops_est", fpops_est)) continue;
        if (xp.parse_double("weight", weight)) continue;
        if (xp.parse_double("working_set", working_set)) continue;
        if (xp.match_tag("fpops")) {
            fpops.parse(xp, "/fpops");
            continue;
        }
        if (xp.parse_int("max_concurrent", max_concurrent)) {
            if (max_concurrent) have_max_concurrent = true;
            continue;
        }
        if (xp.match_tag("checkpoint_period")) {
            checkpoint_period.parse(xp, "/checkpoint_period");
            continue;
        }
#endif
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] APP::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int APP::write(MIOFILE& out) {
    out.printf(
        "<app>\n"
        "    <name>%s</name>\n"
        "    <user_friendly_name>%s</user_friendly_name>\n"
        "    <non_cpu_intensive>%d</non_cpu_intensive>\n"
        "</app>\n",
        name, user_friendly_name,
        non_cpu_intensive?1:0
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
    safe_strcpy(name, "");
    safe_strcpy(md5_cksum, "");
    max_nbytes = 0;
    nbytes = 0;
    gzipped_nbytes = 0;
    upload_offset = -1;
    status = FILE_NOT_PRESENT;
    executable = false;
    uploaded = false;
    sticky = false;
    sticky_lifetime = 0;
    sticky_expire_time = 0;
    gzip_when_done = false;
    ref_cnt = 0;
    download_gzipped = false;
    signature_required = false;
    is_user_file = false;
    is_project_file = false;
    is_auto_update_file = false;
    anonymous_platform_file = false;
    pers_file_xfer = NULL;
    project = NULL;
    download_urls.clear();
    upload_urls.clear();
    safe_strcpy(xml_signature, "");
    safe_strcpy(file_signature, "");
    cert_sigs = 0;
    async_verify = NULL;
}

FILE_INFO::~FILE_INFO() {
#ifndef SIM
    if (async_verify) {
        remove_async_verify(async_verify);
    }
#endif
}

void FILE_INFO::reset() {
    status = FILE_NOT_PRESENT;
    delete_file();
    error_msg.clear();
}

// Set file ownership if using account-based sandbox;
// set permissions depending on whether it's an executable file.
//
// If "path" is non-null, use it instead of the file's
// path in the project directory
// (this is used for files copied into a slot directory)
//
#ifdef _WIN32
int FILE_INFO::set_permissions(const char*) {
    return 0;       // Not relevant in Windows.
}
#else
int FILE_INFO::set_permissions(const char* path) {
    int retval;
    char pathname[1024];
    if (path) {
        safe_strcpy(pathname, path);
    } else {
        get_pathname(this, pathname, sizeof(pathname));
    }

    if (g_use_sandbox) {
        // give exec permissions for user, group and others but give
        // read permissions only for user and group to protect account keys
        retval = set_to_project_group(pathname);
        if (retval) return retval;
        if (executable) {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IWGRP|S_IXGRP
                |S_IXOTH
            );
        } else {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR
                |S_IRGRP|S_IWGRP
            );
        }
    } else {
        // give read/exec permissions for user, group and others
        // in case someone runs BOINC from different user
        if (executable) {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR|S_IXUSR
                |S_IRGRP|S_IXGRP
                |S_IROTH|S_IXOTH
            );
        } else {
            retval = chmod(pathname,
                S_IRUSR|S_IWUSR
                |S_IRGRP
                |S_IROTH
            );
        }
    }
    return retval;
}
#endif

// parse a <file_info>, from state file or scheduler RPC reply
//
int FILE_INFO::parse(XML_PARSER& xp) {
    char buf2[1024];
    string url;
    PERS_FILE_XFER *pfxp;
    int retval;
    bool btemp;
    vector<string>gzipped_urls;

    while (!xp.get_tag()) {
        if (xp.match_tag("/file_info") || xp.match_tag("/file")) {
            if (!strlen(name)) return ERR_BAD_FILENAME;
            if (strstr(name, "..")) return ERR_BAD_FILENAME;
            if (strchr(name, '%')) return ERR_BAD_FILENAME;
            if (gzipped_urls.size() > 0) {
                download_urls.clear();
                download_urls.urls = gzipped_urls;
                download_gzipped = true;
            }
            return 0;
        }
        if (xp.match_tag("xml_signature")) {
            retval = copy_element_contents(
                xp.f->f,
                "</xml_signature>",
                xml_signature,
                sizeof(xml_signature)
            );
            if (retval) return retval;
            strip_whitespace(xml_signature);
            continue;
        }
        if (xp.match_tag("file_signature")) {
            retval = copy_element_contents(
                xp.f->f,
                "</file_signature>",
                file_signature,
                sizeof(file_signature)
            );
            if (retval) return retval;
            strip_whitespace(file_signature);
            continue;
        }
        if (xp.match_tag("signatures")) {
            if (!cert_sigs->parse(xp)) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "FILE_INFO::parse(): cannot parse <signatures>\n"
                );
                return ERR_XML_PARSE;
            }
            continue;
        }

        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_string("url", url)) {
            if (strstr(url.c_str(), "file_upload_handler")) {
                upload_urls.urls.push_back(url);
            } else {
                download_urls.urls.push_back(url);
            }
            continue;
        }
        if (xp.parse_string("download_url", url)) {
            download_urls.urls.push_back(url);
            continue;
        }
        if (xp.parse_string("upload_url", url)) {
            upload_urls.urls.push_back(url);
            continue;
        }
        if (xp.parse_string("gzipped_url", url)) {
            gzipped_urls.push_back(url);
            continue;
        }
        if (xp.parse_str("md5_cksum", md5_cksum, sizeof(md5_cksum))) continue;
        if (xp.parse_double("nbytes", nbytes)) continue;
        if (xp.parse_double("gzipped_nbytes", gzipped_nbytes)) continue;
        if (xp.parse_double("max_nbytes", max_nbytes)) continue;
        if (xp.parse_int("status", status)) {
            // on startup, VERIFY_PENDING is meaningless
            if (status == FILE_VERIFY_PENDING) {
                status = FILE_NOT_PRESENT;
            }
            continue;
        }
        if (xp.parse_bool("executable", executable)) continue;
        if (xp.parse_bool("uploaded", uploaded)) continue;
        if (xp.parse_bool("sticky", sticky)) continue;
        if (xp.parse_double("sticky_expire_time", sticky_expire_time)) continue;
            // state file has this
        if (xp.parse_double("sticky_lifetime", sticky_lifetime)) continue;
            // scheduler RPC reply has this
        if (xp.parse_bool("gzip_when_done", gzip_when_done)) continue;
        if (xp.parse_bool("download_gzipped", download_gzipped)) continue;
        if (xp.parse_bool("signature_required", signature_required)) continue;
        if (xp.parse_bool("is_project_file", is_project_file)) continue;
        if (xp.parse_bool("no_delete", btemp)) continue;
        if (xp.match_tag("persistent_file_xfer")) {
            pfxp = new PERS_FILE_XFER;
            retval = pfxp->parse(xp);
#ifdef SIM
            delete pfxp;
#else
            if (!retval) {
                pers_file_xfer = pfxp;
            } else {
                delete pfxp;
            }
#endif
            continue;
        }
        if (xp.match_tag("file_xfer")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/file_xfer")) break;
            }
            continue;
        }
        if (xp.match_tag("error_msg")) {
            retval = copy_element_contents(
                xp.f->f,
                "</error_msg>", buf2, sizeof(buf2)
            );
            if (retval) return retval;
            error_msg = buf2;
            continue;
        }
        // deprecated tags
        if (xp.parse_bool("generated_locally", btemp)) continue;
        if (xp.parse_bool("upload_when_present", btemp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] FILE_INFO::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    int retval;
    char buf[1024];

    if (to_server) {
        out.printf("<file_info>\n");
    } else {
        out.printf("<file>\n");
    }
    out.printf(
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n",
        name, nbytes, max_nbytes
    );
    if (strlen(md5_cksum)) {
        out.printf(
            "    <md5_cksum>%s</md5_cksum>\n",
            md5_cksum
        );
    }
    if (!to_server) {
        out.printf("    <status>%d</status>\n", status);
        if (executable) out.printf("    <executable/>\n");
        if (uploaded) out.printf("    <uploaded/>\n");
        if (sticky) out.printf("    <sticky/>\n");
        if (gzip_when_done) out.printf("    <gzip_when_done/>\n");
        if (download_gzipped) {
            out.printf("    <download_gzipped/>\n");
            out.printf("    <gzipped_nbytes>%.0f</gzipped_nbytes>\n", gzipped_nbytes);
        }
        if (signature_required) out.printf("    <signature_required/>\n");
        if (is_user_file) out.printf("    <is_user_file/>\n");
        if (strlen(file_signature)) out.printf("    <file_signature>\n%s\n</file_signature>\n", file_signature);
    }
    if (sticky_expire_time) {
        out.printf("    <sticky_expire_time>%f</sticky_expire_time>\n",
            sticky_expire_time
        );
    }
    for (i=0; i<download_urls.urls.size(); i++) {
        xml_escape(download_urls.urls[i].c_str(), buf, sizeof(buf));
        out.printf("    <download_url>%s</download_url>\n", buf);
    }
    for (i=0; i<upload_urls.urls.size(); i++) {
        xml_escape(upload_urls.urls[i].c_str(), buf, sizeof(buf));
        out.printf("    <upload_url>%s</upload_url>\n", buf);
    }
    if (!to_server && pers_file_xfer) {
        retval = pers_file_xfer->write(out);
        if (retval) return retval;
    }
    if (!to_server) {
        if (strlen(xml_signature)) {
            out.printf(
                "    <xml_signature>\n%s    </xml_signature>\n",
                xml_signature
            );
        }
    }
    if (!error_msg.empty()) {
        strip_whitespace(error_msg);
        out.printf("    <error_msg>\n%s\n</error_msg>\n", error_msg.c_str());
    }
    if (to_server) {
        out.printf("</file_info>\n");
    } else {
        out.printf("</file>\n");
    }
    return 0;
}

// called only for files with a PERS_FILE_XFER
//
int FILE_INFO::write_gui(MIOFILE& out) {
    out.printf(
        "<file_transfer>\n"
        "    <project_url>%s</project_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <name>%s</name>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n"
        "    <status>%d</status>\n",
        project->master_url,
        project->project_name,
        name,
        download_gzipped?gzipped_nbytes:nbytes,
        max_nbytes,
        status
    );

    pers_file_xfer->write(out);

    FILE_XFER_BACKOFF& fxb = project->file_xfer_backoff(pers_file_xfer->is_upload);
    if (fxb.next_xfer_time > gstate.now) {
        out.printf("    <project_backoff>%f</project_backoff>\n",
            fxb.next_xfer_time - gstate.now
        );
    }
    out.printf("</file_transfer>\n");
    return 0;
}

// delete physical underlying file associated with FILE_INFO
//
int FILE_INFO::delete_file() {
    char path[MAXPATHLEN];

    get_pathname(this, path, sizeof(path));
    int retval = delete_project_owned_file(path, true);

    // files with download_gzipped set may exist
    // in temporary or compressed form
    //
    safe_strcat(path, ".gz");
    delete_project_owned_file(path, true);
    safe_strcat(path, "t");
    delete_project_owned_file(path, true);

    if (retval && status != FILE_NOT_PRESENT) {
        msg_printf(project, MSG_INTERNAL_ERROR, "Couldn't delete file %s", path);
    }
    status = FILE_NOT_PRESENT;
    return retval;
}

const char* URL_LIST::get_init_url() {
    if (!urls.size()) {
        return NULL;
    }

// if a project supplies multiple URLs, try them in order
// (e.g. in Einstein@home they're ordered by proximity to client).
//
    current_index = 0;
    start_index = current_index;
    return urls[current_index].c_str();
}

// Call this to get the next URL.
// NULL return means you've tried them all.
//
const char* URL_LIST::get_next_url() {
    if (!urls.size()) return NULL;
    while(1) {
        current_index = (current_index + 1)%((int)urls.size());
        if (current_index == start_index) {
            return NULL;
        }
        return urls[current_index].c_str();
    }
}

const char* URL_LIST::get_current_url(FILE_INFO& fi) {
    if (current_index < 0) {
        return get_init_url();
    }
    if (current_index >= (int)urls.size()) {
        msg_printf(fi.project, MSG_INTERNAL_ERROR,
            "File %s has no URL", fi.name
        );
        return NULL;
    }
    return urls[current_index].c_str();
}

// merges information from a new FILE_INFO that has the same name as one
// that is already present in the client state file.
//
int FILE_INFO::merge_info(FILE_INFO& new_info) {
    char buf[256];

    if (max_nbytes <= 0 && new_info.max_nbytes) {
        max_nbytes = new_info.max_nbytes;
        snprintf(buf, sizeof(buf), "    <max_nbytes>%.0f</max_nbytes>\n", new_info.max_nbytes);
    }

    // replace existing URLs with new ones
    //

    download_urls.replace(new_info.download_urls);
    upload_urls.replace(new_info.upload_urls);
    download_gzipped = new_info.download_gzipped;

    // replace signatures
    //
    if (strlen(new_info.file_signature)) {
        safe_strcpy(file_signature, new_info.file_signature);
    }
    if (strlen(new_info.xml_signature)) {
        safe_strcpy(xml_signature, new_info.xml_signature);
    }

    // If the file is supposed to be executable and is PRESENT,
    // make sure it's actually executable.
    // This deals with cases where somehow a file didn't
    // get protected right when it was initially downloaded.
    //
    if (status == FILE_PRESENT && !executable && new_info.executable) {
        msg_printf(project, MSG_INTERNAL_ERROR,
            "%s has changed to executable", name
        );
        executable = true;
        int retval = set_permissions();
        if (retval) {
            msg_printf(project, MSG_INTERNAL_ERROR,
                "merge_info(): failed to change permissions of %s", name
            );
        }
        return retval;
    }

    // sticky attributes
    //
    if (new_info.sticky) {
        sticky = true;
        if (new_info.sticky_lifetime) {
            double x = gstate.now + new_info.sticky_lifetime;
            if (x > sticky_expire_time) {
                sticky_expire_time = x;
            }
        } else {
            sticky_expire_time = 0;
        }
    } else {
        sticky = false;
        sticky_expire_time = 0;
    }

    return 0;
}

// Returns true if the file had an unrecoverable error
// (couldn't download, RSA/MD5 check failed, etc)
//
bool FILE_INFO::had_failure(int& failnum) {
    switch (status) {
    case FILE_NOT_PRESENT:
    case FILE_PRESENT:
    case FILE_VERIFY_PENDING:
        return false;
    }
    failnum = status;
    return true;
}

void FILE_INFO::failure_message(string& s) {
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "<file_xfer_error>\n"
        "  <file_name>%s</file_name>\n"
        "  <error_code>%d (%s)</error_code>\n",
        name,
        status, boincerror(status)
    );
    s = buf;
    if (error_msg.size()) {
        snprintf(buf, sizeof(buf),
            "  <error_message>%s</error_message>\n",
            error_msg.c_str()
            );
        s = s + buf;
    }
    s = s + "</file_xfer_error>\n";
}

#ifndef SIM
#define BUFSIZE 16384
int FILE_INFO::gzip() {
    char buf[BUFSIZE];
    char inpath[MAXPATHLEN], outpath[MAXPATHLEN];

    get_pathname(this, inpath, sizeof(inpath));
    safe_strcpy(outpath, inpath);
    safe_strcat(outpath, ".gz");
    FILE* in = boinc_fopen(inpath, "rb");
    if (!in) return ERR_FOPEN;
    gzFile out = gzopen(outpath, "wb");
    while (1) {
        int n = (int)fread(buf, 1, BUFSIZE, in);
        if (n <= 0) break;
        int m = gzwrite(out, buf, n);
        if (m != n) {
            fclose(in);
            gzclose(out);
            return ERR_WRITE;
        }
    }
    fclose(in);
    gzclose(out);
    delete_project_owned_file(inpath, true);
    boinc_rename(outpath, inpath);
    return 0;
}

// unzip a file, and compute the uncompressed MD5 at the same time
//
int FILE_INFO::gunzip(char* md5_buf) {
    unsigned char buf[BUFSIZE];
    char inpath[MAXPATHLEN], outpath[MAXPATHLEN], tmppath[MAXPATHLEN];
    md5_state_t md5_state;

    md5_init(&md5_state);
    get_pathname(this, outpath, sizeof(outpath));
    safe_strcpy(inpath, outpath);
    safe_strcat(inpath, ".gz");
    safe_strcpy(tmppath, outpath);
    char* p = strrchr(tmppath, '/');
    *(p+1) = 0;
    safe_strcat(tmppath, "decompress_temp");
    FILE* out = boinc_fopen(tmppath, "wb");
    if (!out) return ERR_FOPEN;
    gzFile in = gzopen(inpath, "rb");
    while (1) {
        int n = gzread(in, buf, BUFSIZE);
        if (n <= 0) break;
        int m = (int)fwrite(buf, 1, n, out);
        if (m != n) {
            gzclose(in);
            fclose(out);
            return ERR_WRITE;
        }
        md5_append(&md5_state, buf, n);
    }
    unsigned char binout[16];
    md5_finish(&md5_state, binout);
    for (int i=0; i<16; i++) {
        sprintf(md5_buf+2*i, "%02x", binout[i]);
    }
    md5_buf[32] = 0;

    gzclose(in);
    fclose(out);
    boinc_rename(tmppath, outpath);
    delete_project_owned_file(inpath, true);
    return 0;
}
#endif  // SIM

void RESOURCE_USAGE::clear() {
    avg_ncpus = 0;
    rsc_type = 0;
    coproc_usage = 0;
    gpu_ram = 0;
    flops = 0;
    cmdline[0] = 0;
    missing_coproc = false;
    missing_coproc_name[0] = 0;
}

// see if we have the GPU libraries (OpenCL/CUDA/CAL)
// required by the plan class.
// If not, set missing_coproc
//
void RESOURCE_USAGE::check_gpu_libs(char* plan_class) {
    int rt = rsc_type;
    if (!rt) return;
    if (strstr(plan_class, "opencl")) {
        if (!coprocs.coprocs[rt].have_opencl) {
            msg_printf(0, MSG_INFO,
                "App version needs OpenCL but GPU doesn't support it"
            );
            missing_coproc = true;
            safe_strcpy(missing_coproc_name, coprocs.coprocs[rt].type);
        }
    } else if (strstr(plan_class, "cuda")) {
        if (!coprocs.coprocs[rt].have_cuda) {
            msg_printf(0, MSG_INFO,
                "App version needs CUDA but GPU doesn't support it"
            );
            missing_coproc = true;
            safe_strcpy(missing_coproc_name, coprocs.coprocs[rt].type);
        }
    } else if (strstr(plan_class, "ati")) {
        if (!coprocs.coprocs[rt].have_cal) {
            msg_printf(0, MSG_INFO,
                "App version needs CAL but GPU doesn't support it"
            );
            missing_coproc = true;
            safe_strcpy(missing_coproc_name, coprocs.coprocs[rt].type);
        }
    }
}

void RESOURCE_USAGE::write(MIOFILE& out) {
    out.printf(
        "    <avg_ncpus>%f</avg_ncpus>\n"
        "    <flops>%f</flops>\n",
        avg_ncpus,
        flops
    );
    if (rsc_type) {
        out.printf(
            "    <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            rsc_name(rsc_type),
            coproc_usage
        );
    }
    if (missing_coproc && strlen(missing_coproc_name)) {
        out.printf(
            "    <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            missing_coproc_name,
            coproc_usage
        );
    }
    if (gpu_ram) {
        out.printf(
            "    <gpu_ram>%f</gpu_ram>\n",
            gpu_ram
        );
    }
}

void APP_VERSION::init() {
    app_name[0] = 0;
    version_num = 0;
    platform[0] = 0;
    plan_class[0] = 0;
    api_version[0] = 0;
    resource_usage.clear();
    file_prefix[0] = 0;
    needs_network = false;
    dont_throttle = false;
    app = NULL;
    project = NULL;
    ref_cnt = 0;
    graphics_exec_fip = NULL;
    graphics_exec_path[0] = 0;
    graphics_exec_file[0] = 0;
    max_working_set_size = 0;
    is_vbox_app = false;
    is_docker_app = false;
    is_wrapper = false;
    index = 0;
#ifdef SIM
    dont_use = false;
#endif
}

int APP_VERSION::parse(XML_PARSER& xp) {
    FILE_REF file_ref;
    double dtemp;

    init();
    while (!xp.get_tag()) {
        if (xp.match_tag("/app_version")) {
            dont_throttle = false;
            resource_usage.check_gpu_libs(plan_class);
            if (is_wrapper) {
                // fix problem where wrappers were never throttled
                // can remove this in 8.5 or later
                dont_throttle = false;
            }
            if (resource_usage.rsc_type) {
                // never throttle GPU apps, even if wrapped
                dont_throttle = true;
            }
            if (strstr(plan_class, "vbox")) {
                // VBox does its own throttling
                is_vbox_app = true;
                dont_throttle = true;
            }
            if (strstr(plan_class, "docker")) {
                is_docker_app = true;
            }
            return 0;
        }
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.match_tag("file_ref")) {
            int retval = file_ref.parse(xp);
            if (retval) {
                msg_printf(0, MSG_INFO,
                    "couldn't parse file_ref: %s", boincerror(retval)
                );
                return retval;
            }
            app_files.push_back(file_ref);
            continue;
        }
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_str("api_version", api_version, sizeof(api_version))) continue;
        if (xp.parse_str("platform", platform, sizeof(platform))) continue;
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_double("avg_ncpus", resource_usage.avg_ncpus)) continue;
        if (xp.parse_double("max_ncpus", dtemp)) continue;
        if (xp.parse_double("flops", dtemp)) {
            if (dtemp <= 0) {
                msg_printf(0, MSG_INTERNAL_ERROR,
                    "non-positive FLOPS in app version"
                );
            } else {
                resource_usage.flops = dtemp;
            }
            continue;
        }
        if (xp.parse_str("cmdline", resource_usage.cmdline, sizeof(resource_usage.cmdline))) continue;
        if (xp.parse_str("file_prefix", file_prefix, sizeof(file_prefix))) continue;
        if (xp.parse_double("resource_usage.gpu_ram", resource_usage.gpu_ram)) continue;
        if (xp.match_tag("coproc")) {
            COPROC_REQ cp;
            int retval = cp.parse(xp);
            if (!retval) {
                int rt = rsc_index(cp.type);
                if (rt <= 0) {
                    msg_printf(0, MSG_INFO,
                        "app version refers to missing GPU type %s", cp.type
                    );
                    resource_usage.missing_coproc = true;
                    resource_usage.coproc_usage = cp.count;
                    safe_strcpy(resource_usage.missing_coproc_name, cp.type);
                    continue;
                }
                resource_usage.rsc_type = rt;
                resource_usage.coproc_usage = cp.count;
            } else {
                msg_printf(0, MSG_INTERNAL_ERROR, "Error parsing <coproc>");
            }
            continue;
        }
        if (xp.parse_bool("dont_throttle", dont_throttle)) continue;
        if (xp.parse_bool("is_wrapper", is_wrapper)) continue;
        if (xp.parse_bool("needs_network", needs_network)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] APP_VERSION::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::write(MIOFILE& out, bool write_file_info) {
    unsigned int i;
    int retval;

    out.printf(
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <platform>%s</platform>\n",
        app_name,
        version_num,
        platform
    );
    if (strlen(plan_class)) {
        out.printf("    <plan_class>%s</plan_class>\n", plan_class);
    }
    if (strlen(api_version)) {
        out.printf("    <api_version>%s</api_version>\n", api_version);
    }
    if (strlen(resource_usage.cmdline)) {
        out.printf("    <cmdline>%s</cmdline>\n", resource_usage.cmdline);
    }
    if (strlen(file_prefix)) {
        out.printf("    <file_prefix>%s</file_prefix>\n", file_prefix);
    }
    if (write_file_info) {
        for (i=0; i<app_files.size(); i++) {
            retval = app_files[i].write(out);
            if (retval) return retval;
        }
    }
    resource_usage.write(out);
    if (dont_throttle) {
        out.printf(
            "    <dont_throttle/>\n"
        );
    }
    if (is_wrapper) {
        out.printf(
            "    <is_wrapper/>\n"
        );
    }
    if (needs_network) {
        out.printf(
            "    <needs_network/>\n"
        );
    }

    out.printf(
        "</app_version>\n"
    );
    return 0;
}

bool APP_VERSION::had_download_failure(int& failnum) {
    unsigned int i;

    for (i=0; i<app_files.size();i++) {
        if (app_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void APP_VERSION::get_file_errors(string& str) {
    int errnum;
    unsigned int i;
    FILE_INFO* fip;
    string msg;

    str = "couldn't get input files:\n";
    for (i=0; i<app_files.size();i++) {
        fip = app_files[i].file_info;
        if (fip->had_failure(errnum)) {
            fip->failure_message(msg);
            str = str + msg;
        }
    }
}

void APP_VERSION::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<app_files.size();i++) {
        FILE_INFO* fip = app_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

bool APP_VERSION::api_version_at_least(int major, int minor) {
    int maj, min, n;
    n = sscanf(api_version, "%d.%d", &maj, &min);
    if (n != 2) return false;
    if (maj < major) return false;
    if (maj > major) return true;
    return min >= minor;
}

// If app version has a graphics program,
// see whether the exec is present and can be run.
// If so fill in the file name and path.
// Called from GUI RPC handler.
//
void APP_VERSION::check_graphics_exec() {
    if (!graphics_exec_fip) return;
    if (strlen(graphics_exec_path)) return;
    if (graphics_exec_fip->status < 0) {
        // download or verify of graphics exec failed; don't check again
        //
        graphics_exec_fip = NULL;
        return;
    }
    if (graphics_exec_fip->status != FILE_PRESENT) return;

    char relpath[MAXPATHLEN], path[MAXPATHLEN];
    get_pathname(graphics_exec_fip, relpath, sizeof(relpath));
    relative_to_absolute(relpath, path);
#ifdef __APPLE__
    if (!can_run_on_this_CPU(path)) {
        // if can't run this exec, don't check again
        //
        graphics_exec_fip = NULL;
        return;
    }
#endif
    safe_strcpy(graphics_exec_path, path);
    safe_strcpy(graphics_exec_file, graphics_exec_fip->name);
}

int FILE_REF::parse(XML_PARSER& xp) {
    bool temp;

    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/file_ref")) {
            if (strstr(open_name, "..")) return ERR_BAD_FILENAME;
            if (strchr(open_name, '%')) return ERR_BAD_FILENAME;
            return 0;
        }
        if (xp.parse_str("file_name", file_name, sizeof(file_name))) continue;
        if (xp.parse_str("open_name", open_name, sizeof(open_name))) continue;
        if (xp.parse_bool("main_program", main_program)) continue;
        if (xp.parse_bool("copy_file", copy_file)) continue;
        if (xp.parse_bool("optional", optional)) continue;
        if (xp.parse_bool("no_validate", temp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] FILE_REF::parse(): unrecognized: '%s'\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int FILE_REF::write(MIOFILE& out) {

    out.printf(
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n",
        file_name
    );
    if (strlen(open_name)) {
        out.printf("        <open_name>%s</open_name>\n", open_name);
    }
    if (main_program) {
        out.printf("        <main_program/>\n");
    }
    if (copy_file) {
        out.printf("        <copy_file/>\n");
    }
    if (optional) {
        out.printf("        <optional/>\n");
    }
    out.printf("    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(XML_PARSER& xp) {
    FILE_REF file_ref;
    double dtemp;
    char buf[1024];
    int retval;

    safe_strcpy(name, "");
    safe_strcpy(app_name, "");
    version_num = 0;
    command_line.clear();
    app = NULL;
    project = NULL;
    has_resource_usage = false;
    // Default these to very large values (1 week on a 1 cobblestone machine)
    // so we don't keep asking the server for more work
    rsc_fpops_est = 1e9*SECONDS_PER_DAY*7;
    rsc_fpops_bound = 4e9*SECONDS_PER_DAY*7;
    rsc_memory_bound = 1e8;
    rsc_disk_bound = 1e9;
    while (!xp.get_tag()) {
        if (xp.match_tag("/workunit")) {
            has_resource_usage = resource_usage.avg_ncpus>0
                || resource_usage.rsc_type!=0
                || resource_usage.missing_coproc;
            if (has_resource_usage) {
                resource_usage.check_gpu_libs(plan_class);
            }
            return 0;
        }
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_str("app_name", app_name, sizeof(app_name))) continue;
        if (xp.parse_int("version_num", version_num)) continue;
        if (xp.parse_string("command_line", command_line)) {
            strip_whitespace(command_line);
            continue;
        }
        //if (xp.parse_str("env_vars", env_vars, sizeof(env_vars))) continue;
        if (xp.parse_double("rsc_fpops_est", rsc_fpops_est)) continue;
        if (xp.parse_double("rsc_fpops_bound", rsc_fpops_bound)) continue;
        if (xp.parse_double("rsc_memory_bound", rsc_memory_bound)) continue;
        if (xp.parse_double("rsc_disk_bound", rsc_disk_bound)) continue;
        if (xp.match_tag("file_ref")) {
            retval = file_ref.parse(xp);
            if (retval) {
                msg_printf(0, MSG_INFO,
                    "can't parse file_ref in workunit: %s",
                    boincerror(retval)
                );
                return retval;
            }
#ifndef SIM
            input_files.push_back(file_ref);
#endif
            continue;
        }
        if (xp.parse_str("plan_class", plan_class, sizeof(plan_class))) continue;
        if (xp.parse_str("sub_appname", sub_appname, sizeof(sub_appname))) continue;
        if (xp.parse_double("avg_ncpus", resource_usage.avg_ncpus)) continue;
        if (xp.parse_double("flops", dtemp)) {
            if (dtemp <= 0) {
                msg_printf(0, MSG_INTERNAL_ERROR, "non-positive FLOPS in WU");
            } else {
                resource_usage.flops = dtemp;
            }
            continue;
        }
        if (xp.parse_str("cmdline", resource_usage.cmdline, sizeof(resource_usage.cmdline))) continue;
        if (xp.parse_double("resource_usage.gpu_ram", resource_usage.gpu_ram)) continue;
        if (xp.match_tag("coproc")) {
            COPROC_REQ cp;
            retval = cp.parse(xp);
            if (!retval) {
                int rt = rsc_index(cp.type);
                if (rt <= 0) {
                    msg_printf(0, MSG_INFO,
                        "WU refers to missing GPU type %s", cp.type
                    );
                    resource_usage.missing_coproc = true;
                    resource_usage.coproc_usage = cp.count;
                    safe_strcpy(resource_usage.missing_coproc_name, cp.type);
                    continue;
                }
                resource_usage.rsc_type = rt;
                resource_usage.coproc_usage = cp.count;
            } else {
                msg_printf(0, MSG_INTERNAL_ERROR, "Error parsing <coproc>");
            }
            continue;
        }

        if (xp.parse_str("job_keyword_ids", buf, sizeof(buf))) {
            job_keyword_ids.parse_str(buf );
            continue;
        }
        // unused stuff
        if (xp.parse_double("credit", dtemp)) continue;
        if (log_flags.unparsed_xml) {
            msg_printf(0, MSG_INFO,
                "[unparsed_xml] WORKUNIT::parse(): unrecognized: %s\n",
                xp.parsed_tag
            );
        }
        xp.skip_unexpected();
    }
    return ERR_XML_PARSE;
}

int WORKUNIT::write(MIOFILE& out, bool gui) {
    unsigned int i;

    out.printf(
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        //"    <env_vars>%s</env_vars>\n"
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n",
        name,
        app_name,
        version_num,
        //env_vars,
        rsc_fpops_est,
        rsc_fpops_bound,
        rsc_memory_bound,
        rsc_disk_bound
    );
    if (command_line.size()) {
        out.printf(
            "    <command_line>\n"
            "%s\n"
            "    </command_line>\n",
            command_line.c_str()
        );
    }
    for (i=0; i<input_files.size(); i++) {
        input_files[i].write(out);
    }
    if (strlen(sub_appname)) {
        out.printf(
            "    <sub_appname>%s</sub_appname>\n",
            sub_appname
        );
    }
    if (resource_usage.present()) {
        resource_usage.write(out);
    }

    if (!job_keyword_ids.empty()) {
        if (gui) {
            if (gstate.keywords.present) {
                job_keyword_ids.write_xml_text(out, gstate.keywords);
            }
        } else {
            job_keyword_ids.write_xml_num(out);
        }
    }
    out.printf("</workunit>\n");
    return 0;
}

bool WORKUNIT::had_download_failure(int& failnum) {
    unsigned int i;

    for (i=0;i<input_files.size();i++) {
        if (input_files[i].file_info->had_failure(failnum)) {
            return true;
        }
    }
    return false;
}

void WORKUNIT::get_file_errors(string& str) {
    int x;
    unsigned int i;
    FILE_INFO* fip;
    string msg;

    str = "couldn't get input files:\n";
    for (i=0;i<input_files.size();i++) {
        fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->failure_message(msg);
            str = str + msg;
        }
    }
}

// if any input files had download error from previous WU,
// reset them to try download again
//
void WORKUNIT::clear_errors() {
    int x;
    unsigned int i;
    for (i=0; i<input_files.size();i++) {
        FILE_INFO* fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            fip->reset();
        }
    }
}

RUN_MODE::RUN_MODE() {
    perm_mode = 0;
    temp_mode = 0;
    prev_mode = 0;
    temp_timeout = 0;
}

void RUN_MODE::set(int mode, double duration) {
    if (mode == 0) mode = RUN_MODE_AUTO;
    if (mode == RUN_MODE_RESTORE) {
        temp_timeout = 0;
        if (temp_mode == perm_mode) {
            perm_mode = prev_mode;
        }
        temp_mode = perm_mode;
        return;
    }
    prev_mode = temp_mode;
    if (duration) {
        temp_mode = mode;
        temp_timeout = gstate.now + duration;
    } else {
        temp_timeout = 0;
        temp_mode = mode;
        perm_mode = mode;
        gstate.set_client_state_dirty("Set mode");
    }

    // In case we read older state file with no prev_mode
    if (prev_mode == 0) prev_mode = temp_mode;
}

void RUN_MODE::set_prev(int mode) {
    prev_mode = mode;
}

int RUN_MODE::get_perm() {
    return perm_mode;
}

int RUN_MODE::get_prev() {
    return prev_mode;
}

int RUN_MODE::get_current() {
    if (temp_timeout > gstate.now) {
        return temp_mode;
    } else {
        return perm_mode;
    }
}

double RUN_MODE::delay() {
    if (temp_timeout > gstate.now) {
        return temp_timeout - gstate.now;
    } else {
        return 0;
    }
}

int USER_CPID::parse(XML_PARSER& xp) {
    clear();
    while (!xp.get_tag()) {
        if (xp.match_tag("/user_cpid")) {
            if (!strlen(email_hash) || !strlen(cpid) || !time) {
                msg_printf(0, MSG_INTERNAL_ERROR, "USER_CPID::parse() failed");
                return ERR_XML_PARSE;
            }
            break;
        }
        if (xp.parse_str("email_hash", email_hash, sizeof(email_hash))) continue;
        if (xp.parse_str("cpid", cpid, sizeof(cpid))) continue;
        if (xp.parse_double("time", time)) continue;
    }
    return 0;
}

int USER_CPID::write(MIOFILE& out) {
    out.printf(
        "   <user_cpid>\n"
        "      <email_hash>%s</email_hash>\n"
        "      <cpid>%s</cpid>\n"
        "      <time>%f</time>\n"
        "   </user_cpid>\n",
        email_hash, cpid, time
    );
    return 0;
}

int USER_CPIDS::parse(XML_PARSER& xp) {
    while (!xp.get_tag()) {
        if (xp.match_tag("/user_cpids")) {
            break;
        }
        if (xp.match_tag("user_cpid")) {
            USER_CPID uc;
            int retval = uc.parse(xp);
            if (retval) continue;
            cpids.push_back(uc);
        }
    }
    return 0;
}

int USER_CPIDS::write(MIOFILE& out) {
    out.printf("<user_cpids>\n");
    for (USER_CPID &cpid: cpids) {
        cpid.write(out);
    }
    out.printf("</user_cpids>\n");
    return 0;
}

USER_CPID* USER_CPIDS::lookup(const char* email_hash) {
    for (USER_CPID &cpid: cpids) {
        if (!strcmp(cpid.email_hash, email_hash)) {
            return &cpid;
        }
    }
    return NULL;
}

// if empty, initialize from projects
// (should get done once, when updating to this client version)
//
void USER_CPIDS::init_from_projects() {
    for (PROJECT *p: gstate.projects) {
        if (!strlen(p->email_hash) || !strlen(p->cross_project_id) || !p->user_create_time) {
            continue;
        }
        USER_CPID* ucp = lookup(p->email_hash);
        if (ucp) {
            if (p->user_create_time < ucp->time) {
                strcpy(ucp->cpid, p->cross_project_id);
                ucp->time = p->user_create_time;
            }
        } else {
            USER_CPID uc;
            strcpy(uc.email_hash, p->email_hash);
            strcpy(uc.cpid, p->cross_project_id);
            uc.time = p->user_create_time;
            cpids.push_back(uc);

        }
    }
}

USER_CPIDS user_cpids;
