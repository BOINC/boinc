// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "client_msgs.h"
#include "log_flags.h"
#include "parse.h"
#include "util.h"
#include "client_state.h"
#include "pers_file_xfer.h"

#include "client_types.h"

using std::string;

PROJECT::PROJECT() {
    init();
}

void PROJECT::init() {
    strcpy(master_url, "");
    strcpy(authenticator, "");
    project_specific_prefs = "";
    resource_share = 100;
    strcpy(project_name, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    user_total_credit = 0;
    user_expavg_credit = 0;
    user_create_time = 0;
    rpc_seqno = 0;
    hostid = 0;
    host_total_credit = 0;
    host_expavg_credit = 0;
    host_create_time = 0;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    strcpy(code_sign_key, "");
    nrpc_failures = 0;
    min_rpc_time = 0;
    min_report_min_rpc_time = 0;
    master_fetch_failures = 0;
    resource_debt = 0;
    debt_order = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
    tentative = false;
	anonymous_platform = false;
    debt = 0;
    anticipated_debt = 0;
    work_done_this_period = 0;
    next_runnable_result = NULL;
    work_request = 0;
	send_file_list = false;
}

PROJECT::~PROJECT() {
}

// write account_*.xml file.
// NOTE: this is called only when
// 1) attach to a project, and
// 2) after a scheduler RPC
// So in either case PROJECT.project_prefs
// (which normally is undefined) is valid
//
int PROJECT::write_account_file() {
    char path[256];
    FILE* f;
    int retval;

    get_account_filename(master_url, path);
    f = boinc_fopen(TEMP_FILE_NAME, "w");
    if (!f) return ERR_FOPEN;
#ifndef _WIN32
    chmod(TEMP_FILE_NAME, 0600);
#endif

    fprintf(f,
        "<account>\n"
        "    <master_url>%s</master_url>\n"
        "    <authenticator>%s</authenticator>\n",
        master_url,
        authenticator
    );
    // put project name in account file for informational purposes only
    // (client state file is authoritative)
    //
    if (strlen(project_name)) {
        fprintf(f, "    <project_name>%s</project_name>\n", project_name);
    }
    if (tentative) {
        fprintf(f, "    <tentative/>\n");
    }
    fprintf(f, "<project_preferences>\n%s</project_preferences>\n",
        project_prefs.c_str()
    );
    fprintf(f, "</account>\n");
    fclose(f);
    retval = boinc_rename(TEMP_FILE_NAME, path);
    if (retval) return ERR_RENAME;
    return 0;
}

int PROJECT::parse_account_file() {
    char path[256];
    int retval;
    FILE* f;

    get_account_filename(master_url, path);
    f = fopen(path, "r");
    if (!f) return ERR_FOPEN;
    retval = parse_account(f);
    fclose(f);
    return retval;
}

// parse user's project preferences, generating
// FILE_REF and FILE_INFO objects for each <app_file> element.
//
int PROJECT::parse_preferences_for_user_files() {
    char* p, *q, *q2;
    char buf[1024];
    string timestamp, open_name, url, filename;
    FILE_INFO* fip;
    FILE_REF fr;
    STRING256 url_str;
    char prefs_buf[MAX_BLOB_LEN];
    strcpy(prefs_buf, project_specific_prefs.c_str());
    p = prefs_buf;

    user_files.clear();
    while (1) {
        q = strstr(p, "<app_file>");
        if (!q) break;
        q2 = strstr(q, "</app_file>");
        if (!q2) break;
        *q2 = 0;
        strcpy(buf, q);
        if (!parse_str(buf, "<timestamp>", timestamp)) break;
        if (!parse_str(buf, "<open_name>", open_name)) break;
        if (!parse_str(buf, "<url>", url)) break;
        strcpy(url_str.text, url.c_str());

        filename = open_name + "_" + timestamp;
        fip = gstate.lookup_file_info(this, filename.c_str());
        if (!fip) {
            fip = new FILE_INFO;
            fip->urls.push_back(url_str);
            strcpy(fip->name, filename.c_str());
            fip->project = this;
            fip->is_user_file = true;
            gstate.file_infos.push_back(fip);
        }

        fr.file_info = fip;
        strcpy(fr.open_name, open_name.c_str());
        user_files.push_back(fr);

        p = q2+strlen("</app_file>");
    }

    return 0;
}

// parse account_*.xml file; fill in files of
// project_specific_prefs
//
int PROJECT::parse_account(FILE* in) {
    char buf[256], venue[256];
    char temp[MAX_BLOB_LEN];
    int retval;
    bool got_venue_prefs = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    // Assume master_url_fetch_pending, sched_rpc_pending are
    // true until we read client_state.xml
    //
    master_url_fetch_pending = true;
    sched_rpc_pending = true;
    strcpy(master_url, "");
    strcpy(authenticator, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "<account>")) continue;
        if (match_tag(buf, "<project_preferences>")) continue;
        if (match_tag(buf, "</project_preferences>")) continue;
        if (match_tag(buf, "</venue>")) continue;
        if (match_tag(buf, "</account>")) {
            if (strlen(gstate.host_venue)) {
                if (!got_venue_prefs) {
                    msg_printf(this, MSG_INFO,
                        "Project prefs: no separate prefs for %s; using your defaults",
                        gstate.host_venue
                    );
                }
            } else {
                msg_printf(this, MSG_INFO, "Project prefs: using your defaults");
            }
            return 0;
        }

        else if (match_tag(buf, "<venue")) {
            parse_attr(buf, "name", venue, sizeof(venue));
            if (!strcmp(venue, gstate.host_venue)) {
                msg_printf(this, MSG_INFO,
                    "Project prefs: using separate prefs for %s",
                    gstate.host_venue
                );
                got_venue_prefs = true;
            } else {
                copy_element_contents(in, "</venue>", temp, sizeof(temp));
            }
            continue;
        }

        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) {
            canonicalize_master_url(master_url);
            continue;
        }
        else if (parse_str(buf, "<authenticator>", authenticator, sizeof(authenticator))) continue;
        else if (parse_double(buf, "<resource_share>", resource_share)) continue;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        else if (match_tag(buf, "<tentative/>")) {
            tentative = true;
            continue;
        }
        else if (match_tag(buf, "<project_specific>")) {
            retval = copy_element_contents(
                in,
                "</project_specific>",
                project_specific_prefs
            );
            if (retval) return ERR_XML_PARSE;
            continue;
        }
        else scope_messages.printf("PROJECT::parse_account(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// parse project fields from client_state.xml
//
int PROJECT::parse_state(MIOFILE& in) {
    char buf[256];
    STRING256 sched_url;
    string str1, str2;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(project_name, "");
    strcpy(user_name, "");
    strcpy(team_name, "");
    strcpy(email_hash, "");
    strcpy(cross_project_id, "");
    resource_share = 100;
    exp_avg_cpu = 0;
    exp_avg_mod_time = 0;
    min_rpc_time = 0;
    min_report_min_rpc_time = 0;
    nrpc_failures = 0;
    master_url_fetch_pending = false;
    sched_rpc_pending = false;
	send_file_list = false;
    scheduler_urls.clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</project>")) return 0;
        else if (parse_str(buf, "<scheduler_url>", sched_url.text, sizeof(sched_url.text))) {
            scheduler_urls.push_back(sched_url);
            continue;
        }
        else if (parse_str(buf, "<master_url>", master_url, sizeof(master_url))) continue;
        else if (parse_str(buf, "<project_name>", project_name, sizeof(project_name))) continue;
        else if (parse_str(buf, "<user_name>", user_name, sizeof(user_name))) continue;
        else if (parse_str(buf, "<team_name>", team_name, sizeof(team_name))) continue;
        else if (parse_str(buf, "<email_hash>", email_hash, sizeof(email_hash))) continue;
        else if (parse_str(buf, "<cross_project_id>", cross_project_id, sizeof(cross_project_id))) continue;
        else if (parse_double(buf, "<user_total_credit>", user_total_credit)) continue;
        else if (parse_double(buf, "<user_expavg_credit>", user_expavg_credit)) continue;
        else if (parse_int(buf, "<user_create_time>", (int &)user_create_time)) continue;
        else if (parse_int(buf, "<rpc_seqno>", rpc_seqno)) continue;
        else if (parse_int(buf, "<hostid>", hostid)) continue;
        else if (parse_double(buf, "<host_total_credit>", host_total_credit)) continue;
        else if (parse_double(buf, "<host_expavg_credit>", host_expavg_credit)) continue;
        else if (parse_int(buf, "<host_create_time>", (int &)host_create_time)) continue;
        else if (parse_double(buf, "<exp_avg_cpu>", exp_avg_cpu)) continue;
        else if (parse_double(buf, "<exp_avg_mod_time>", exp_avg_mod_time)) continue;
        else if (match_tag(buf, "<code_sign_key>")) {
            copy_element_contents(
                in,
                "</code_sign_key>",
                code_sign_key,
                sizeof(code_sign_key)
            );
        }
        else if (parse_int(buf, "<nrpc_failures>", nrpc_failures)) continue;
        else if (parse_int(buf, "<master_fetch_failures>", master_fetch_failures)) continue;
        else if (parse_int(buf, "<min_rpc_time>", (int&)min_rpc_time)) continue;
        else if (match_tag(buf, "<master_url_fetch_pending/>")) master_url_fetch_pending = true;
        else if (match_tag(buf, "<sched_rpc_pending/>")) sched_rpc_pending = true;
		else if (match_tag(buf, "<send_file_list/>")) send_file_list = true;
        else if (parse_double(buf, "<debt>", debt)) continue;
        else scope_messages.printf("PROJECT::parse_state(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// Write the project information to client state file
//
int PROJECT::write_state(MIOFILE& out) {
    unsigned int i;
    string u1, u2, t1, t2;

    out.printf(
        "<project>\n"
    );
    for (i=0; i<scheduler_urls.size(); i++) {
        out.printf(
            "    <scheduler_url>%s</scheduler_url>\n",
            scheduler_urls[i].text
        );
    }
    u1 = user_name;
    xml_escape(u1, u2);
    t1 = team_name;
    xml_escape(t1, t2);
    out.printf(
        "    <master_url>%s</master_url>\n"
        "    <project_name>%s</project_name>\n"
        "    <user_name>%s</user_name>\n"
        "    <team_name>%s</team_name>\n"
        "    <email_hash>%s</email_hash>\n"
        "    <cross_project_id>%s</cross_project_id>\n"
        "    <user_total_credit>%f</user_total_credit>\n"
        "    <user_expavg_credit>%f</user_expavg_credit>\n"
        "    <user_create_time>%d</user_create_time>\n"
        "    <rpc_seqno>%d</rpc_seqno>\n"
        "    <hostid>%d</hostid>\n"
        "    <host_total_credit>%f</host_total_credit>\n"
        "    <host_expavg_credit>%f</host_expavg_credit>\n"
        "    <host_create_time>%d</host_create_time>\n"
        "    <exp_avg_cpu>%f</exp_avg_cpu>\n"
        "    <exp_avg_mod_time>%f</exp_avg_mod_time>\n"
        "    <nrpc_failures>%d</nrpc_failures>\n"
        "    <master_fetch_failures>%d</master_fetch_failures>\n"
        "    <min_rpc_time>%d</min_rpc_time>\n"
        "    <debt>%f</debt>\n"
        "%s%s\n",
        master_url,
        project_name,
        u2.c_str(),
        t2.c_str(),
        email_hash,
        cross_project_id,
        user_total_credit,
        user_expavg_credit,
        user_create_time,
        rpc_seqno,
        hostid,
        host_total_credit,
        host_expavg_credit,
        host_create_time,
        exp_avg_cpu,
        exp_avg_mod_time,
        nrpc_failures,
        master_fetch_failures,
        (int)min_rpc_time,
        debt,
        master_url_fetch_pending?"    <master_url_fetch_pending/>\n":"",
        sched_rpc_pending?"    <sched_rpc_pending/>\n":"",
		send_file_list?"     <send_file_list/>\n":""
    );
    if (strlen(code_sign_key)) {
        out.printf(
            "    <code_sign_key>\n%s</code_sign_key>\n", code_sign_key
        );
    }
    out.printf(
        "</project>\n"
    );
    return 0;
}

// copy fields from "p" into "this" that are stored in client_state.xml
//
void PROJECT::copy_state_fields(PROJECT& p) {
    scheduler_urls = p.scheduler_urls;
    safe_strcpy(project_name, p.project_name);
    safe_strcpy(user_name, p.user_name);
    safe_strcpy(team_name, p.team_name);
    safe_strcpy(email_hash, p.email_hash);
    safe_strcpy(cross_project_id, p.cross_project_id);
    user_total_credit = p.user_total_credit;
    user_expavg_credit = p.user_expavg_credit;
    user_create_time = p.user_create_time;
    rpc_seqno = p.rpc_seqno;
    hostid = p.hostid;
    host_total_credit = p.host_total_credit;
    host_expavg_credit = p.host_expavg_credit;
    host_create_time = p.host_create_time;
    exp_avg_cpu = p.exp_avg_cpu;
    exp_avg_mod_time = p.exp_avg_mod_time;
    nrpc_failures = p.nrpc_failures;
    master_fetch_failures = p.master_fetch_failures;
    min_rpc_time = p.min_rpc_time;
    master_url_fetch_pending = p.master_url_fetch_pending;
    sched_rpc_pending = p.sched_rpc_pending;
    safe_strcpy(code_sign_key, p.code_sign_key);
    debt = p.debt;
}

char* PROJECT::get_project_name() {
    if (strlen(project_name)) {
        return project_name;
    } else {
        return master_url;
    }
}

int APP::parse(MIOFILE& in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(name, "");
    project = NULL;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else scope_messages.printf("APP::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP::write(MIOFILE& out) {
    out.printf(
        "<app>\n"
        "    <name>%s</name>\n"
        "</app>\n",
        name
    );
    return 0;
}

FILE_INFO::FILE_INFO() {
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
    is_user_file = false;
    pers_file_xfer = NULL;
    result = NULL;
    project = NULL;
    urls.clear();
    start_url = -1;
    current_url = -1;
    strcpy(signed_xml, "");
    strcpy(xml_signature, "");
    strcpy(file_signature, "");
}

FILE_INFO::~FILE_INFO() {
    if (pers_file_xfer) {
		msg_printf(NULL, MSG_ERROR, "FILE_INFO::~FILE_INFO(): removing FILE_INFO when a pers_file_xfer still points to me\n");
        pers_file_xfer->fip = NULL;
    }
}

// Set the appropriate permissions depending on whether
// it's an executable file
// This doesn't seem to exist in Windows
//
int FILE_INFO::set_permissions() {
#ifdef _WIN32
    return 0;
#else
    int retval;
    char pathname[256];
    get_pathname(this, pathname);
    if (executable) {
        retval = chmod(pathname, S_IEXEC|S_IREAD|S_IWRITE);
    } else {
        retval = chmod(pathname, S_IREAD|S_IWRITE);
    }
    return retval;
#endif
}

// see if a file markes as present actually IS present
// and have the right size.
// If not, mark it as not present
//
bool FILE_INFO::verify_existing_file() {
    int retval;
    double size;
    char path[256];

    get_pathname(this, path);
    retval = file_size(path, size);
    if (retval) {
        status = FILE_NOT_PRESENT;
        return false;
    }
    if (!log_flags.dont_check_file_sizes && size!=nbytes) {
        status = FILE_NOT_PRESENT;
        return false;
    }
    return true;
}

// If from server, make an exact copy of everything
// except the start/end tags and the <xml_signature> element.
//
int FILE_INFO::parse(MIOFILE& in, bool from_server) {
    char buf[256], buf2[1024];
    STRING256 url;
    PERS_FILE_XFER *pfxp;
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_info>")) return 0;
        else if (match_tag(buf, "<xml_signature>")) {
            copy_element_contents(
                in,
                "</xml_signature>",
                xml_signature,
                sizeof(xml_signature)
            );
            continue;
        }
        if (from_server) {
            strcat(signed_xml, buf);
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else if (parse_str(buf, "<url>", url.text, sizeof(url.text))) {
            urls.push_back(url);
            continue;
        }
        else if (match_tag(buf, "<file_signature>")) {
            copy_element_contents(
                in,
                "</file_signature>",
                file_signature,
                sizeof(file_signature)
            );
            continue;
        }
        else if (parse_str(buf, "<md5_cksum>", md5_cksum, sizeof(md5_cksum))) continue;
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
        } else if (!from_server && match_tag(buf, "<signed_xml>")) {
            copy_element_contents(
                in,
                "</signed_xml>",
                signed_xml,
                sizeof(signed_xml)
            );
            continue;
   	    } else if (match_tag(buf, "<file_xfer>")) {
   	    	while (in.fgets(buf, 256)) {
   	    		if (match_tag(buf, "</file_xfer>")) break;
   	   	    }
   	   	    continue;
        } else if (match_tag(buf, "<error_msg>")) {
            copy_element_contents(in, "</error_msg>", buf2, sizeof(buf2));
            error_msg = buf2;
        } else scope_messages.printf("FILE_INFO::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int FILE_INFO::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    int retval;

    out.printf(
        "<file_info>\n"
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
        if (generated_locally) out.printf("    <generated_locally/>\n");
        out.printf("    <status>%d</status>\n", status);
        if (executable) out.printf("    <executable/>\n");
        if (uploaded) out.printf("    <uploaded/>\n");
        if (upload_when_present) out.printf("    <upload_when_present/>\n");
        if (sticky) out.printf("    <sticky/>\n");
        if (signature_required) out.printf("    <signature_required/>\n");
        if (file_signature) out.printf("    <file_signature>\n%s</file_signature>\n", file_signature);
    }
    for (i=0; i<urls.size(); i++) {
        out.printf("    <url>%s</url>\n", urls[i].text);
    }
    if (!to_server && pers_file_xfer) {
        retval = pers_file_xfer->write(out);
        if (retval) return retval;
    }
    if (!to_server) {
        if (strlen(signed_xml)) {
            out.printf("    <signed_xml>\n%s    </signed_xml>\n", signed_xml);
        }
        if (strlen(xml_signature)) {
            out.printf("    <xml_signature>\n%s    </xml_signature>\n", xml_signature);
        }
    }
    if (!error_msg.empty()) {
        out.printf("    <error_msg>\n%s</error_msg>\n", error_msg.c_str());
    }
    out.printf("</file_info>\n");
    return 0;
}

// delete physical underlying file associated with FILE_INFO
//
int FILE_INFO::delete_file() {
    char path[256];

    get_pathname(this, path);
    int retval = boinc_delete_file(path);
	if (retval && status != FILE_NOT_PRESENT) {
        msg_printf(project, MSG_ERROR, "Couldn't delete file %s\n", path);
	}
    status = FILE_NOT_PRESENT;
	return retval;
}

// If a file has multiple replicas, we want to choose
// a random one to try first, and then cycle through others
// if transfers fail.
// Call this to get the initial url,
//
// Files may have URLs for both upload and download.
// The is_upload arg says which kind you want.
// NULL return means there is no URL of the requested type
//
char* FILE_INFO::get_init_url(bool is_upload) {
    double temp;
    temp = rand();
    temp *= urls.size();
    temp /= RAND_MAX;
    current_url = (int)temp;
	start_url = current_url;
	while(1) {
		if(!is_correct_url_type(is_upload, urls[current_url])) {
			current_url = (current_url + 1)%urls.size();
			if (current_url == start_url) {
				msg_printf(project, MSG_ERROR, "Couldn't find suitable url for %s\n", name);
				return NULL;
			}
		} else {
			start_url = current_url;
			return urls[current_url].text;
		}
	}
}

// Call this to get the next URL of the indicated type.
// NULL return means you've tried them all.
//
char* FILE_INFO::get_next_url(bool is_upload) {
	while(1) {
	    current_url = (current_url + 1)%urls.size();
		if (current_url == start_url) {
			return NULL;
		}
		if(is_correct_url_type(is_upload, urls[current_url])) {
			return urls[current_url].text;
		}
	}
}

char* FILE_INFO::get_current_url(bool is_upload) {
	if (current_url < 0) {
		return get_init_url(is_upload);
	}
	return urls[current_url].text;
}

// Checks if the url includes the phrase "file_upload_handler"
// The inclusion of this phrase indicates the url is an upload url
// 
bool FILE_INFO::is_correct_url_type(bool is_upload, STRING256 url) {
	if(is_upload && !strstr(url.text, "file_upload_handler") ||
		!is_upload && strstr(url.text, "file_upload_handler")) {
		return false;
	} else {
		return true;
	}
}

// merges information from a new FILE_INFO that has the same name as a 
// FILE_INFO that is already present in the client state
// Potentially changes upload_when_present, max_nbytes, and signed_xml
//
int FILE_INFO::merge_info(FILE_INFO& new_info) {
	char buf[256];
	bool has_url;
	unsigned int i, j;
	upload_when_present = new_info.upload_when_present;
	if(max_nbytes <= 0 && new_info.max_nbytes) {
		max_nbytes = new_info.max_nbytes;
		sprintf(buf, "    <max_nbytes>%.0f</max_nbytes>\n", new_info.max_nbytes);
		strcat(signed_xml, buf);
	}
	for(i = 0; i < new_info.urls.size(); i++) {
		has_url = false;
		for(j = 0; j < urls.size(); j++) {
			if(!strcmp(urls[j].text, new_info.urls[i].text)) {
				has_url = true;
			}
		}
		if(!has_url) {
			urls.push_back(new_info.urls[i]);
		}
	}
	return 0;
}

// Returns true if the file had an unrecoverable error
// (couldn't download, RSA/MD5 check failed, etc)
//
bool FILE_INFO::had_failure(int& failnum) {
    if (status != FILE_NOT_PRESENT && status != FILE_PRESENT) {
        failnum = status;
        return true;
    }
    return false;
}

// Parse XML based app_version information, usually from client_state.xml
//
int APP_VERSION::parse(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(app_name, "");
    version_num = 0;
    app = NULL;
    project = NULL;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</app_version>")) return 0;
        else if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            app_files.push_back(file_ref);
            continue;
        }
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else scope_messages.printf("APP_VERSION::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int APP_VERSION::write(MIOFILE& out) {
    unsigned int i;
    int retval;

    out.printf(
        "<app_version>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n",
        app_name,
        version_num
    );
    for (i=0; i<app_files.size(); i++) {
        retval = app_files[i].write(out);
        if (retval) return retval;
    }
    out.printf(
        "</app_version>\n"
    );
    return 0;
}

int FILE_REF::parse(MIOFILE& in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(file_name, "");
    strcpy(open_name, "");
    fd = -1;
    main_program = false;
	copy_file = false;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</file_ref>")) return 0;
        else if (parse_str(buf, "<file_name>", file_name, sizeof(file_name))) continue;
        else if (parse_str(buf, "<open_name>", open_name, sizeof(open_name))) continue;
        else if (parse_int(buf, "<fd>", fd)) continue;
        else if (match_tag(buf, "<main_program/>")) main_program = true;
        else if (match_tag(buf, "<copy_file/>")) copy_file = true;
        else scope_messages.printf("FILE_REF::parse(): unrecognized: %s\n", buf);
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
    if (fd >= 0) {
        out.printf("        <fd>%d</fd>\n", fd);
    }
    if (main_program) {
        out.printf("        <main_program/>\n");
    }
    if (copy_file) {
        out.printf("        <copy_file/>\n");
    }
    out.printf("    </file_ref>\n");
    return 0;
}

int WORKUNIT::parse(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(name, "");
    strcpy(app_name, "");
    version_num = 0;
    strcpy(command_line, "");
    strcpy(env_vars, "");
    app = NULL;
    project = NULL;
    // Default these to very large values (1 week on a 1 cobblestone machine)
    // so we don't keep asking the server for more work
    rsc_fpops_est = 1e9*SECONDS_PER_DAY*7;
    rsc_fpops_bound = 4e9*SECONDS_PER_DAY*7;
    rsc_memory_bound = 1e8;
    rsc_disk_bound = 1e9;
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</workunit>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else if (parse_str(buf, "<app_name>", app_name, sizeof(app_name))) continue;
        else if (parse_int(buf, "<version_num>", version_num)) continue;
        else if (parse_str(buf, "<command_line>", command_line, sizeof(command_line))) continue;
        else if (parse_str(buf, "<env_vars>", env_vars, sizeof(env_vars))) continue;
        else if (parse_double(buf, "<rsc_fpops_est>", rsc_fpops_est)) continue;
        else if (parse_double(buf, "<rsc_fpops_bound>", rsc_fpops_bound)) continue;
        else if (parse_double(buf, "<rsc_memory_bound>", rsc_memory_bound)) continue;
        else if (parse_double(buf, "<rsc_disk_bound>", rsc_disk_bound)) continue;
        else if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            input_files.push_back(file_ref);
            continue;
        }
        else scope_messages.printf("WORKUNIT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int WORKUNIT::write(MIOFILE& out) {
    unsigned int i;

    out.printf(
        "<workunit>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n"
        "    <version_num>%d</version_num>\n"
        "    <command_line>%s</command_line>\n"
        "    <env_vars>%s</env_vars>\n"
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n",
        name,
        app_name,
        version_num,
        command_line,
        env_vars,
        rsc_fpops_est,
        rsc_fpops_bound,
        rsc_memory_bound,
        rsc_disk_bound
    );
    for (i=0; i<input_files.size(); i++) {
        input_files[i].write(out);
    }
    out.printf("</workunit>\n");
    return 0;
}

bool WORKUNIT::had_failure(int& failnum) {
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
    str = "couldn't get input files:\n";
    for (i=0;i<input_files.size();i++) {
        fip = input_files[i].file_info;
        if (fip->had_failure(x)) {
            str = str + fip->name + ": " + fip->error_msg + "\n";
        }
    }
}

int RESULT::parse_ack(FILE* in) {
    char buf[256];

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    strcpy(name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</result_ack>")) return 0;
        else if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

void RESULT::clear() {
    strcpy(name, "");
    strcpy(wu_name, "");
    report_deadline = 0;
    output_files.clear();
    is_active = false;
    state = RESULT_NEW;
    ready_to_report = false;
    got_server_ack = false;
    final_cpu_time = 0;
    exit_status = 0;
    active_task_state = 0;
    signal = 0;
    stderr_out = "";
    app = NULL;
    wup = NULL;
    project = NULL;
}

// parse a <result> element from scheduling server.
//
int RESULT::parse_server(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) return 0;
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

// parse a <result> element from state file
//
int RESULT::parse_state(MIOFILE& in) {
    char buf[256];
    FILE_REF file_ref;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);

    clear();
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</result>")) {
            // restore some invariants in case of bad state file
            //
            if (got_server_ack || ready_to_report) {
                state = RESULT_FILES_UPLOADED;
            }
            return 0;
        }
        if (parse_str(buf, "<name>", name, sizeof(name))) continue;
        if (parse_str(buf, "<wu_name>", wu_name, sizeof(wu_name))) continue;
        if (parse_int(buf, "<report_deadline>", report_deadline)) continue;
        if (match_tag(buf, "<file_ref>")) {
            file_ref.parse(in);
            output_files.push_back(file_ref);
            continue;
        }
        else if (parse_double(buf, "<final_cpu_time>", final_cpu_time)) continue;
        else if (parse_int(buf, "<exit_status>", exit_status)) continue;
        else if (match_tag(buf, "<got_server_ack/>")) got_server_ack = true;
        else if (match_tag(buf, "<ready_to_report/>")) ready_to_report = true;
        else if (parse_int(buf, "<state>", state)) continue;
        else if (match_tag(buf, "<stderr_out>")) {
            while (in.fgets(buf, 256)) {
                if (match_tag(buf, "</stderr_out>")) break;
                stderr_out.append(buf);
            }
            continue;
        }
        else scope_messages.printf("RESULT::parse(): unrecognized: %s\n", buf);
    }
    return ERR_XML_PARSE;
}

int RESULT::write(MIOFILE& out, bool to_server) {
    unsigned int i;
    FILE_INFO* fip;
    int n, retval;

    out.printf(
        "<result>\n"
        "    <name>%s</name>\n"
        "    <final_cpu_time>%f</final_cpu_time>\n"
        "    <exit_status>%d</exit_status>\n"
        "    <state>%d</state>\n",
        name,
        final_cpu_time,
        exit_status,
        state
    );
    if (to_server) {
        out.printf(
            "    <app_version_num>%d</app_version_num>\n",
            wup->version_num
        );
    }
    n = stderr_out.length();
    if (n) {
        out.printf("<stderr_out>\n");
        if (to_server) {
            out.printf(
                "<core_client_version>%d.%.2d</core_client_version>\n",
                gstate.core_client_major_version,
                gstate.core_client_minor_version
            );
        }
		out.printf(stderr_out.c_str());
        if (stderr_out[n-1] != '\n') {
            out.printf("\n");
        }
        out.printf("</stderr_out>\n");
    }
    if (to_server) {
        for (i=0; i<output_files.size(); i++) {
            fip = output_files[i].file_info;
            if (fip->uploaded) {
                retval = fip->write(out, true);
                if (retval) return retval;
            }
        }
    } else {
        if (got_server_ack) out.printf("    <got_server_ack/>\n");
        if (ready_to_report) out.printf("    <ready_to_report/>\n");
        out.printf(
            "    <wu_name>%s</wu_name>\n"
            "    <report_deadline>%d</report_deadline>\n",
            wu_name,
            report_deadline
        );
        for (i=0; i<output_files.size(); i++) {
            retval = output_files[i].write(out);
            if (retval) return retval;
        }
    }
    out.printf("</result>\n");
    return 0;
}

// this is called after the result state is RESULT_COMPUTE_DONE.
// Returns true if the result's output files are all either
// successfully uploaded or have unrecoverable errors
//
bool RESULT::is_upload_done() {
    unsigned int i;
    FILE_INFO* fip;
    int retval;

    for (i=0; i<output_files.size(); i++) {
        fip = output_files[i].file_info;
        if (fip->upload_when_present) {
            if (fip->had_failure(retval)) continue;
            if (!fip->uploaded) {
                return false;
            }
        }
    }
    return true;
}

void RESULT::get_app_version_string(string& str) {
    char buf[256];
    sprintf(buf, " %.2f", wup->version_num/100.);
    str = app->name + string(buf);
}

// resets all FILE_INFO's in result to uploaded = false
// used in file_xfers when an uploaded file is required
// without calling this before sending result to be uploaded,
// upload would terminate without sending files

void RESULT::reset_result_files() {
	unsigned int i;

	for (i=0; i<output_files.size(); i++) {
		output_files[i].file_info->uploaded = false;
	}
}
