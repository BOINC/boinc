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

#include "windows_cpp.h"

#include <sys/stat.h>
#include <sys/types.h>

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "pers_file_xfer.h"

#include "client_types.h"

PROJECT::PROJECT() {
    strcpy(master_url,"");
    strcpy(authenticator,"");
    project_specific_prefs = NULL;
    resource_share = 0;
    strcpy(project_name,"");
    strcpy(user_name,"");
    rpc_seqno = 0;
    hostid = 0;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    code_sign_key = NULL;
    nrpc_failures = 0;
    min_rpc_time = 0;
    resource_debt = 0;
    debt_order = 0;
    master_url_fetch_pending = 0;
}

PROJECT::~PROJECT() {
    if (project_specific_prefs) free(project_specific_prefs);
    if (code_sign_key) free(code_sign_key);
}

// parse project fields from prefs.xml
//
int PROJECT::parse_prefs(FILE* in) {
    char buf[256], *p;
    int retval;

    strcpy(master_url, "");
    strcpy(authenticator, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<master_url>", master_url)) continue;
        else if (parse_str(buf, "<authenticator>", authenticator)) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (match_tag(buf, "<project_specific>")) {
            retval = dup_element_contents(in, "</project_specific>", &p);
            if (retval) return ERR_XML_PARSE;
            project_specific_prefs = p;
            continue;
        }
        else fprintf(stderr, "PROJECT::parse_prefs(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// parse project fields from client_state.xml
//
int PROJECT::parse_state(FILE* in) {
    char buf[256];
    STRING256 string;

    strcpy(project_name, "");
    strcpy(user_name, "");
    resource_share = 1;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    min_rpc_time = 0;
    nrpc_failures = 0;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<scheduler_url>", string.text)) {
            scheduler_urls.push_back(string);
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url)) continue;
        else if (parse_str(buf, "<project_name>", project_name)) continue;
        else if (parse_str(buf, "<user_name>", user_name)) continue;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<exp_avg_cpu>", exp_avg_cpu)) continue;
        else if (parse_int(buf, "<exp_avg_mod_time>", exp_avg_mod_time)) continue;
        else if (match_tag(buf, "<code_sign_key>")) {
            dup_element_contents(in, "</code_sign_key>", &code_sign_key);
            //fprintf(stderr, "code_sign_key: %s\n", code_sign_key);
        }
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<min_rpc_time>", min_rpc_time)) continue;
        else fprintf(stderr, "PROJECT::parse_state(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write the project information to prefs.xml
//
int PROJECT::write_state(FILE* out) {
    unsigned int i;

    fprintf(out,
        "<project>\n"
    );
    for (i=0; i<scheduler_urls.size(); i++) {
        fprintf(out,
            "    <scheduler_url>%s</scheduler_url>\n",
            scheduler_urls[i].text
        );
    }
    fprintf(out,
        "    <master_url>%s</master_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <user_name>%s</user_name>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <hostid>%d</hostid>\n"
        "    <exp_avg_cpu>%f</exp_avg_cpu>\n"
        "    <exp_avg_mod_time>%d</exp_avg_mod_time>\n"
        "    <nrpc_failures>%d</nrpc_failures>\n"
        "    <min_rpc_time>%d</min_rpc_time>\n",
        master_url,
        project_name,
        user_name,
        rpc_seqno,
        hostid,
        exp_avg_cpu,
        exp_avg_mod_time,
        nrpc_failures,
        min_rpc_time
    );
    if (code_sign_key) {
        fprintf(out,
            "    <code_sign_key>\n%s</code_sign_key>\n", code_sign_key
        );
    }
    fprintf(out,
        "</project>\n"
    );
    return 0;
}

// copy fields from "p" into "this" that are stored in client_state.xml
//
void PROJECT::copy_state_fields(PROJECT& p) {
    scheduler_urls = p.scheduler_urls;
    strcpy( project_name, p.project_name );
    strcpy( user_name, p.user_name );
    rpc_seqno = p.rpc_seqno;
    hostid = p.hostid;
    exp_avg_cpu = p.exp_avg_cpu;
    exp_avg_mod_time = p.exp_avg_mod_time;
    if( p.code_sign_key ) {
        code_sign_key = strdup(p.code_sign_key);
    }
    nrpc_failures = p.nrpc_failures;
    min_rpc_time = p.min_rpc_time;
}

void PROJECT::copy_prefs_fields(PROJECT& p) {
    strcpy( authenticator, p.authenticator );
    if (p.project_specific_prefs) {
        project_specific_prefs = strdup(p.project_specific_prefs);
    }
    resource_share = p.resource_share;
}

int APP::parse(FILE* in) {
    char buf[256];

    strcpy(name, "");
    project = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else fprintf(stderr, "APP::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP::write(FILE* out) {
    fprintf(out,
        "<app>\n"
        "    <name>%s</name>\n"
        "</app>\n",
        name
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
}

FILE_INFO::~FILE_INFO() {
}

// Set the appropriate permissions depending on whether
// it's an executable or normal file
// TODO: implement Windows equivalent
int FILE_INFO::set_permissions() {
    int retval;
    char pathname[256];

    get_pathname(this, pathname);
#ifndef _WIN32
    if (executable) {
        retval = chmod(pathname, S_IEXEC|S_IREAD|S_IWRITE);
    } else {
        retval = chmod(pathname, S_IREAD|S_IWRITE);
    }
#endif
    return retval;
}

// If from server, make an exact copy of everything
// except the start/end tags and the <xml_signature> element.
//
int FILE_INFO::parse(FILE* in, bool from_server) {
    char buf[256];
    STRING256 url;
    PERS_FILE_XFER *pfxp;
    int retval;

    strcpy(name, "");
    strcpy(md5_cksum, "");
    max_nbytes = 0;
    nbytes = 0;
    upload_offset = -1;
    generated_locally = false;
    status = FILE_NOT_PRESENT;
    executable = false;
    uploaded = false;
    upload_when_present = false;
    sticky = false;
    signature_required = false;
    pers_file_xfer = NULL;
    result = NULL;
    project = NULL;
    urls.clear();
    start_url = -1;
    current_url = -1;
    if (from_server) {
        signed_xml = strdup("");
    } else {
        signed_xml = NULL;
    }
    xml_signature = NULL;
    file_signature = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</file_info>")) return 0;
        else if (match_tag(buf, "<xml_signature>")) {
            dup_element_contents(in, "</xml_signature>", &xml_signature);
            continue;
        }
        if (from_server) {
            strcatdup(signed_xml, buf);
        }
        if (parse_str(buf, "<name>", name)) continue;
        else if (parse_str(buf, "<url>", url.text)) {
            urls.push_back(url);
            continue;
        }
        else if (match_tag(buf, "<file_signature>")) {
            dup_element_contents(in, "</file_signature>", &file_signature);
            continue;
        }
        else if (parse_str(buf, "<md5_cksum>", md5_cksum)) continue;
        else if (parse_double(buf, "<nbytes>", nbytes)) continue;
        else if (parse_double(buf, "<max_nbytes>", max_nbytes)) continue;
        else if (match_tag(buf, "<generated_locally/>")) generated_locally = true;
        else if (parse_int(buf, "<status>", status)) continue;
        else if (match_tag(buf, "<executable/>")) executable = true;
        else if (match_tag(buf, "<uploaded/>")) uploaded = true;
        else if (match_tag(buf, "<upload_when_present/>")) upload_when_present = true;
        else if (match_tag(buf, "<sticky/>")) sticky = true;
        else if (match_tag(buf, "<signature_required/>")) signature_required = true;
        else if (match_tag(buf, "<persistent_file_xfer>")) {
            pfxp = new PERS_FILE_XFER;
            retval = pfxp->parse(in);
            if (!retval) {
                pers_file_xfer = pfxp;
            } else {
                delete pfxp;
            }
        }
        else if (!from_server && match_tag(buf, "<signed_xml>")) {
            dup_element_contents(in, "</signed_xml>", &signed_xml);
            continue;
        }
        else fprintf(stderr, "FILE_INFO::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

// Write out XML based file info
//
int FILE_INFO::write(FILE* out, bool to_server) {
    unsigned int i;

    fprintf(out,
        "<file_info>\n"
        "    <name>%s</name>\n"
        "    <md5_cksum>%s</md5_cksum>\n"
        "    <nbytes>%f</nbytes>\n"
        "    <max_nbytes>%f</max_nbytes>\n",
        name, md5_cksum, nbytes, max_nbytes
    );
    if (!to_server) {
        if (generated_locally) fprintf(out, "    <generated_locally/>\n");
        fprintf(out, "    <status>%d</status>\n", status);
        if (executable) fprintf(out, "    <executable/>\n");
        if (uploaded) fprintf(out, "    <uploaded/>\n");
        if (upload_when_present) fprintf(out, "    <upload_when_present/>\n");
        if (sticky) fprintf(out, "    <sticky/>\n");
        if (signature_required) fprintf(out, "    <signature_required/>\n");
    }
    for (i=0; i<urls.size(); i++) {
        fprintf(out, "<url>%s</url>\n", urls[i].text);
    }
    if (!to_server && pers_file_xfer) {
        pers_file_xfer->write(out);
    }
    if (!to_server) {
        if (signed_xml) {
            fprintf(out, "<signed_xml>\n%s</signed_xml>\n", signed_xml);
        }
        if (xml_signature) {
            fprintf(out, "<xml_signature>\n%s</xml_signature>\n", xml_signature);
        }
    }
    fprintf(out, "</file_info>\n");
    return 0;
}

// delete physical underlying file associated with FILE_INFO
//
int FILE_INFO::delete_file() {
    char path[256];

    get_pathname(this, path);
    status = FILE_NOT_PRESENT;
    return file_delete(path);
}

// get the currently selected url to download/upload file, or
// select one if none is chosen yet
//
char* FILE_INFO::get_url() {
    double temp;
    if( current_url < 0 ) {
        temp = rand();
        temp *= urls.size();
        temp /= RAND_MAX;
        current_url = (int)temp;
        start_url = current_url;
    }

    return urls[current_url].text;
}

// Returns true if the file had some sort of error
// (couldn't download, RSA/MD5 check failed, etc)
//
bool FILE_INFO::had_failure() {
    return (status != FILE_NOT_PRESENT && status != FILE_PRESENT);
}

// Parse XML based app_version information, usually from client_state.xml
//
int APP_VERSION::parse(FILE* in) {
    char buf[256];
    FILE_REF file_ref;

    strcpy(app_name, "");
    version_num = 0;
    app = NULL;
    project = NULL;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name)) continue;
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            app_files.push_back(file_ref);
            continue;
        }
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else fprintf(stderr, "APP_VERSION::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int APP_VERSION::write(FILE* out) {
    unsigned int i;

    fprintf(out,
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n",
        app_name,
        version_num
    );
    for (i=0; i<app_files.size(); i++) {
        app_files[i].write(out);
    }
    fprintf(out,
        "</app_version>\n"
    );
    return 0;
}

int FILE_REF::parse(FILE* in) {
    char buf[256];

    strcpy(file_name, "");
    strcpy(open_name, "");
    fd = -1;
    main_program = false;
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</file_ref>")) return 0;
        else if (parse_str(buf, "<file_name>", file_name)) continue;
        else if (parse_str(buf, "<open_name>", open_name)) continue;
        else if (parse_int(buf, "<fd>", fd)) continue;
        else if (match_tag(buf, "<main_program/>")) main_program = true;
        else fprintf(stderr, "FILE_REF::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

int FILE_REF::write(FILE* out) {

    fprintf(out,
        "    <file_ref>\n"
        "        <file_name>%s</file_name>\n",
        file_name
    );
    if (strlen(open_name)) {
        fprintf(out, "        <open_name>%s</open_name>\n", open_name);
    }
    if (fd >= 0) {
        fprintf(out, "        <fd>%d</fd>\n", fd);
    }
    if (main_program) {
        fprintf(out, "        <main_program/>\n");
    }
    fprintf(out, "    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(FILE* in) {
    char buf[256];
    FILE_REF file_ref;

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
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else if (parse_str(buf, "<command_line>", command_line)) continue;
        else if (parse_str(buf, "<env_vars>", env_vars)) continue;
	else if (parse_double(buf, "<seconds_to_complete>", 
			      seconds_to_complete)) continue; 
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            input_files.push_back(file_ref);
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
        input_files[i].write(out);
    }
    fprintf(out, "</workunit>\n");
    return 0;
}

int RESULT::parse_ack(FILE* in) {
    char buf[256];

    strcpy(name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</result_ack>")) return 0;
        else if (parse_str(buf, "<name>", name)) continue;
        else fprintf(stderr, "RESULT::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    report_deadline = 0;
    output_files.clear();
    is_active = false;
    state = RESULT_NEW;
    final_cpu_time = 0;
    exit_status = 0;
    strcpy(stderr_out, "");
    app = NULL;
    wup = NULL;
    project = NULL;
}

// parse a <result> element from scheduling server.
//
int RESULT::parse_server(FILE* in) {
    char buf[256];
    FILE_REF file_ref;

    clear();
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</result>")) return 0;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<wu_name>", wu_name)) continue;
        if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else fprintf(stderr, "RESULT::parse(): unrecognized: %s\n", buf);
    }
    return 1;
}

// parse a <result> element from state file
//
int RESULT::parse_state(FILE* in) {
    char buf[256];
    FILE_REF file_ref;

    clear();
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</result>")) return 0;
        if (parse_str(buf, "<name>", name)) continue;
        if (parse_str(buf, "<wu_name>", wu_name)) continue;
        if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (parse_int(buf, "<state>", state)) continue;
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
        "    <final_cpu_time>%f</final_cpu_time>\n",
        name,
        exit_status,
        final_cpu_time
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
            "    <state>%d</state>\n"
            "    <wu_name>%s</wu_name>\n"
            "    <report_deadline>%d</report_deadline>\n",
            state,
            wu_name,
            report_deadline
        );
        for (i=0; i<output_files.size(); i++) {
            output_files[i].write(out);
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

// this is called only after the result state reaches
// COMPUTE_DONE is true.  Returns true if the result
// and it's associated files were successfully uploaded
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
