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

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"

int PROJECT::parse(FILE* in) {
    char buf[256];

    strcpy(name, "");
    strcpy(domain, "");
    strcpy(scheduler_url, "");
    strcpy(url_source, "");
    strcpy(email_addr, "");
    strcpy(authenticator, "");
    home_project = false;
    next_request_time = 0;
    resource_share = 1;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<domain>", domain)) continue;
        else if (parse_str(buf, "<scheduler_url>", scheduler_url)) continue;
        else if (parse_str(buf, "<url_source>", url_source)) continue;
        else if (parse_str(buf, "<email_addr>", email_addr)) continue;
        else if (parse_str(buf, "<authenticator>", authenticator)) continue;
        else if (match_tag(buf, "<home_project/>")) home_project = true;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_int(buf, "<next_request_time>", next_request_time)) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_double(buf, "<exp_avg_cpu>", exp_avg_cpu)) continue;
        else if (parse_int(buf, "<exp_avg_mod_time>", exp_avg_mod_time)) continue;
        else fprintf(stderr, "PROJECT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int PROJECT::write(FILE* out) {
    fprintf(out,
        "<project>\n"
        "    <name>%s</name>\n"
        "    <domain>%s</domain>\n"
        "    <scheduler_url>%s</scheduler_url>\n"
        "    <url_source>%s</url_source>\n"
        "    <email_addr>%s</email_addr>\n"
        "    <authenticator>%s</authenticator>\n",
        name, domain, scheduler_url, url_source, email_addr, authenticator
    );
    if (home_project) fprintf(out, "    <home_project/>\n");
    fprintf(out,
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <hostid>%d</hostid>\n"
        "    <next_request_time>%d</next_request_time>\n"
        "    <resource_share>%f</resource_share>\n"
        "    <exp_avg_cpu>%f</exp_avg_cpu>\n"
        "    <exp_avg_mod_time>%d</exp_avg_mod_time>\n"
        "</project>\n",
        rpc_seqno,
        hostid,
        next_request_time,
        resource_share,
        exp_avg_cpu,
        exp_avg_mod_time
    );
    return 0;
}

int APP::parse(FILE* in) {
    char buf[256];

    strcpy(name, "");
    project = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else fprintf(stderr, "APP::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP::write(FILE* out) {
    fprintf(out,
        "<app>\n"
        "    <name>%s</name>\n"
        "    <version_num>%d</version_num>\n"
        "</app>\n",
        name,
        version_num
    );
    return 0;
}

int FILE_INFO::parse(FILE* in) {
    char buf[256];

    strcpy(name, "");
    strcpy(url, "");
    strcpy(md5_cksum, "");
    nbytes = 0;
    generated_locally = false;
    file_present = false;
    executable = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    project = NULL;
    file_xfer = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</file_info>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<url>", url)) continue;
        else if (parse_str(buf, "<md5_cksum>", md5_cksum)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (match_tag(buf, "<generated_locally/>")) generated_locally = true;
        else if (match_tag(buf, "<file_present/>")) file_present = true;
        else if (match_tag(buf, "<executable/>")) executable = true;
        else if (match_tag(buf, "<uploaded/>")) uploaded = true;
        else if (match_tag(buf, "<upload_when_present/>")) upload_when_present = true;
        else if (match_tag(buf, "<sticky/>")) sticky = true;
        else fprintf(stderr, "FILE_INFO::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int FILE_INFO::write(FILE* out, bool to_server) {
    fprintf(out,
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <url>%s</url>\n"
        "    <md5_cksum>%s</md5_cksum>\n"
        "    <nbytes>%f</nbytes>\n",
        name, url, md5_cksum, nbytes
    );
    if (!to_server) {
        if (generated_locally) fprintf(out, "    <generated_locally/>\n");
        if (file_present) fprintf(out, "    <file_present/>\n");
        if (executable) fprintf(out, "    <executable/>\n");
        if (uploaded) fprintf(out, "    <uploaded/>\n");
        if (upload_when_present) fprintf(out, "    <upload_when_present/>\n");
        if (sticky) fprintf(out, "    <sticky/>\n");
    }
    fprintf(out, "</file_info>\n");
    return 0;
}

// delete underlying file
//
int FILE_INFO::delete_file() {
    char path[256];

    get_pathname(this, path);
    return file_delete(path);
}

int APP_VERSION::parse(FILE* in) {
    char buf[256];

    strcpy(app_name, "");
    strcpy(file_name, "");
    version_num = 0;
    app = NULL;
    project = NULL;
    file_info = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_str(buf, "<file_name>", file_name)) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else fprintf(stderr, "APP_VERSION::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int APP_VERSION::write(FILE* out) {
    fprintf(out,
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <file_name>%s</file_name>\n"
        "    <version_num>%d</version_num>\n"
        "</app_version>\n",
        app_name, file_name, version_num
    );
    return 0;
}

int IO_FILE_DESC::parse(FILE* in, char* end_tag) {
    char buf[256];

    strcpy(file_name, "");
    strcpy(open_name, "");
    fd = -1;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, end_tag)) return 0;
        else if (parse_str(buf, "<file_name>", file_name)) continue;
        else if (parse_str(buf, "<open_name>", open_name)) continue;
        else if (parse_int(buf, "<fd>", fd)) continue;
        else fprintf(stderr, "IO_FILE_DESC::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int IO_FILE_DESC::write(FILE* out, char* tag) {
    fprintf(out, "    <%s>\n", tag);
    if (strlen(open_name)) {
        fprintf(out, "        <open_name>%s</open_name>\n", open_name);
    }
    fprintf(out,
        "        <file_name>%s</file_name>\n",
        file_name
    );
    if (fd >= 0) {
        fprintf(out, "        <fd>%d</fd>\n", fd);
    }
    fprintf(out, "    </%s>\n", tag);
    return 0;
}

int WORKUNIT::parse(FILE* in) {
    char buf[256];
    IO_FILE_DESC ifd;

    strcpy(name, "");
    strcpy(app_name, "");
    version_num = 0;
    strcpy(command_line, "");
    strcpy(env_vars, "");
    app = NULL;
    project = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</workunit>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (parse_str(buf, "<command_line>", command_line)) continue;
        else if (parse_str(buf, "<env_vars>", env_vars)) continue;
        else if (match_tag(buf, "<input_file>")) {
            ifd.parse(in, "</input_file>");
            input_files.push_back(ifd);
            continue;
        }
        else fprintf(stderr, "WORKUNIT::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int WORKUNIT::write(FILE* out) {
    unsigned int i;
    fprintf(out,
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <command_line>%s</command_line>\n"
        "    <env_vars>%s</env_vars>\n",
        name, app_name, version_num, command_line, env_vars
    );
    for (i=0; i<input_files.size(); i++) {
        input_files[i].write(out, "input_file");
    }
    fprintf(out, "</workunit>\n");
    return 0;
}

int RESULT::parse(FILE* in, char* end_tag) {
    char buf[256];
    IO_FILE_DESC ifd;

    strcpy(name, "");
    strcpy(wu_name, "");
    strcpy(stderr_out, "");
    is_active = false;
    is_compute_done = false;
    is_server_ack = false;
    cpu_time = 0;
    exit_status = 0;
    app = NULL;
    wup = NULL;
    project = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, end_tag)) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<wu_name>", wu_name)) continue;
        else if (match_tag(buf, "<output_file>")) {
            ifd.parse(in, "</output_file>");
            output_files.push_back(ifd);
            continue;
        }
        else if (parse_double(buf, "<cpu_time>", cpu_time)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (match_tag(buf, "<stderr_out>" )) {
            while (fgets(buf, 256, in)) {
                if (match_tag(buf, "</stderr_out>")) break;
                strcat(stderr_out, buf);
            }
            continue;
        }
        else fprintf(stderr, "RESULT::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int RESULT::write(FILE* out, bool to_server) {
    unsigned int i;
    FILE_INFO* fip;
    int n;

    fprintf(out,
        "<result>\n"
        "    <name>%s</name>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <cpu_time>%f</cpu_time>\n",
        name,
        exit_status,
        cpu_time
    );
    n = strlen(stderr_out);
    if (n) {
        fprintf(out,
            "<stderr_out>\n"
            "%s",
            stderr_out
        );
        if (stderr_out[n-1] != '\n') fprintf(out, "\n");
        fprintf(out,
            "</stderr_out>\n"
        );
    }
    if (!to_server) {
        fprintf(out,
            "    <wu_name>%s</wu_name>\n",
            wu_name
        );
        for (i=0; i<output_files.size(); i++) {
            output_files[i].write(out, "output_file");
        }
    } else {
        for (i=0; i<output_files.size(); i++) {
            fip = output_files[i].file_info;
            if (fip->uploaded) {
                fip->write(out, to_server);
            }
        }
    }
    fprintf(out, "</result>\n");
    return 0;
}

// this is called only after is_compute_done is true.
//
bool RESULT::is_upload_done() {
    unsigned int i;
    FILE_INFO* fip;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present && !fip->uploaded) {
            return false;
        }
    }
    return true;
}
