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

// C++ interfaces to web RPCs related to remote job submission,
// namely those described here:
// http://boinc.berkeley.edu/trac/wiki/RemoteInputFiles
// http://boinc.berkeley.edu/trac/wiki/RemoteOutputFiles
// http://boinc.berkeley.edu/trac/wiki/RemoteJobs

#include <curl/curl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>

#include "parse.h"

#include "remote_submit.h"

using std::vector;
using std::string;

//#define SHOW_REPLY

// do an HTTP GET request.
//
static int do_http_get(
    const char* url,
    const char* dst_path
) {
    FILE* reply = fopen(dst_path, "w");
    if (!reply) return -1;
    CURL *curl = curl_easy_init();
    if (!curl) {
        fclose(reply);
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BOINC remote job submission");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reply);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    fclose(reply);
    return 0;
}

// send an HTTP POST request,
// with an optional set of multi-part file attachments
//
static int do_http_post(
    const char* url,
    const char* request,
    FILE* reply,
    vector<string> send_files
) {
    CURL *curl;
    CURLcode res;
    char buf[256];
     
    curl = curl_easy_init();
    if (!curl) {
        return -1;
    }

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;

    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "request",
        CURLFORM_COPYCONTENTS, request,
        CURLFORM_END
    );
    for (unsigned int i=0; i<send_files.size(); i++) {
        sprintf(buf, "file_%d", i);
        string s = send_files[i];
        curl_formadd(&formpost, &lastptr,
            CURLFORM_COPYNAME, buf,
            CURLFORM_FILE, s.c_str(),
            CURLFORM_END
        );
    }
 
    headerlist = curl_slist_append(headerlist, "Expect:");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "BOINC Condor adapter");
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_READDATA, request);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reply);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL error: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    curl_slist_free_all(headerlist);
    return 0;
}

int query_files(
    const char* project_url,
    const char* authenticator,
    vector<string> &paths,
    vector<string> &md5s,
    int batch_id,
    vector<int> &absent_files,
    string& error_msg
) {
    string req_msg;
    char buf[256];
    req_msg = "<query_files>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    req_msg += string(buf);
    if (batch_id) {
        sprintf(buf, "<batch_id>%d</batch_id>\n", batch_id);
        req_msg += string(buf);
    }
    for (unsigned int i=0; i<md5s.size(); i++) {
        sprintf(buf, "   <md5>%s</md5>\n", md5s[i].c_str());
        req_msg += string(buf);
    }
    req_msg += "</query_files>\n";
    FILE* reply = tmpfile();
    char url[256];
    sprintf(url, "%sjob_file.php", project_url);
    int retval = do_http_post(url, req_msg.c_str(), reply, paths);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    int x;
    retval = -1;
    error_msg = "";
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("query_files reply: %s", buf);
#endif
        if (strstr(buf, "absent_files")) {
            retval = 0;
            continue;
        }
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (parse_int(buf, "<file>", x)) {
            absent_files.push_back(x);
            continue;
        }
    }
    fclose(reply);
    return retval;
}

int upload_files (
    const char* project_url,
    const char* authenticator,
    vector<string> &paths,
    vector<string> &md5s,
    int batch_id,
    string &error_msg
) {
    char buf[1024];
    string req_msg = "<upload_files>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    req_msg += string(buf);
    if (batch_id) {
        sprintf(buf, "<batch_id>%d</batch_id>\n", batch_id);
        req_msg += string(buf);
    }
    for (unsigned int i=0; i<md5s.size(); i++) {
        sprintf(buf, "<md5>%s</md5>\n", md5s[i].c_str());
        req_msg += string(buf);
    }
    req_msg += "</upload_files>\n";
    FILE* reply = tmpfile();
    char url[256];
    sprintf(url, "%sjob_file.php", project_url);
    int retval = do_http_post(url, req_msg.c_str(), reply, paths);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    bool success = false;
    retval = -1;
    error_msg = "";
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("upload_files reply: %s", buf);
#endif
        if (strstr(buf, "success")) {
            retval = 0;
            continue;
        }
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
    }
    fclose(reply);
    return retval;
}

int create_batch(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    const char* app_name,
    int& batch_id,
    string& error_msg
) {
    char request[1024];
    char url[1024];
    sprintf(request,
        "<create_batch>\n"
        "   <authenticator>%s</authenticator>\n"
        "      <batch>\n"
        "         <batch_name>%s</batch_name>\n"
        "         <app_name>%s</app_name>\n"
        "      </batch>\n"
        "</create_batch>\n",
        authenticator,
        batch_name,
        app_name
    );
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request, reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    char buf[256];
    batch_id = 0;
    fseek(reply, 0, SEEK_SET);
    int error_num = 0;
    error_msg = "";
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("create_batch reply: %s", buf);
#endif
        if (parse_int(buf, "<batch_id>", batch_id)) continue;
        if (parse_int(buf, "<error_num>", error_num)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;

    }
    fclose(reply);
    return error_num;
}

int submit_jobs(
    const char* project_url,
    const char* authenticator,
    SUBMIT_REQ &req,
    string& error_msg
) {
    char buf[1024], url[1024];
    sprintf(buf,
        "<submit_batch>\n"
        "<authenticator>%s</authenticator>\n"
        "<batch>\n"
        "   <batch_id>%d</batch_id>\n"
        "   <app_name>%s</app_name>\n",
        authenticator,
        req.batch_id,
        req.app_name
    );
    string request = buf;
    for (unsigned int i=0; i<req.jobs.size(); i++) {
        JOB job = req.jobs[i];
        request += "<job>\n";
        sprintf(buf, "  <name>%s</name>\n", job.job_name);
        request += buf;
        if (!job.cmdline_args.empty()) {
            request += "<command_line>" + job.cmdline_args + "</command_line>\n";
        }
        for (unsigned int j=0; j<job.infiles.size(); j++) {
            INFILE infile = job.infiles[j];
            map<string, LOCAL_FILE>::iterator iter = req.local_files.find(infile.src_path);
            if (iter == req.local_files.end()) {
                fprintf(stderr, "file %s not in map\n", infile.src_path);
                exit(1);
            }
            LOCAL_FILE& lf = iter->second;
            sprintf(buf,
                "<input_file>\n"
                "<mode>local_staged</mode>\n"
                "<source>jf_%s</source>\n"
                "</input_file>\n",
                lf.md5
            );
            request += buf;
        }
        request += "</job>\n";
    }
    request += "</batch>\n</submit_batch>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    error_msg = "";
    int temp;
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("submit_batch reply: %s", buf);
#endif
        if (parse_int(buf, "<batch_id>", temp)) {
            retval = 0;
            continue;
        }
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
    }
    fclose(reply);
    return retval;
}

int query_batches(
    const char* project_url,
    const char* authenticator,
    vector<string> &batch_names,
    QUERY_BATCH_REPLY& qb_reply,
    string& error_msg
) {
    string request;
    char url[1024], buf[256];
    int batch_size;

    request = "<query_batch2>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    for (unsigned int i=0; i<batch_names.size(); i++) {
        sprintf(buf, "<batch_name>%s</batch_name>\n", batch_names[i].c_str());
        request += string(buf);
    }
    request += "</query_batch2>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    error_msg = "";
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("query_batches reply: %s", buf);
#endif
        if (strstr(buf, "jobs")) {
            retval = 0;
            continue;
        }
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (parse_int(buf, "<batch_size>", batch_size)) {
            qb_reply.batch_sizes.push_back(batch_size);
            continue;
        }
        if (strstr(buf, "<job>")) {
            QUERY_BATCH_JOB qbj;
            while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
                printf("query_batches reply: %s", buf);
#endif
                if (strstr(buf, "</job>")) {
                    qb_reply.jobs.push_back(qbj);
                    break;
                }
                if (parse_str(buf, "job_name", qbj.job_name)) continue;
                if (parse_str(buf, "status", qbj.status)) continue;
            }
            continue;
        }
    }
    fclose(reply);
    return retval;
}

int abort_jobs(
    const char* project_url,
    const char* authenticator,
    vector<string> &job_names,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<abort_jobs>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    for (unsigned int i=0; i<job_names.size(); i++) {
        sprintf(buf, "<job_name>%s</job_name>\n", job_names[i].c_str());
        request += string(buf);
    }
    request += "</abort_jobs>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    error_msg = "";
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("abort_jobs reply: %s", buf);
#endif
        if (strstr(buf, "success")) {
            retval = 0;
            continue;
        }
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
    }
    fclose(reply);
    return retval;
}

int get_templates(
    const char* project_url,
    const char* authenticator,
    const char* app_name,
    const char* job_name,
    TEMPLATE_DESC &td,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<get_templates>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    if (app_name) {
        sprintf(buf, "<app_name>%s</app_name>\n", app_name);
        request += string(buf);
    } else {
        sprintf(buf, "<job_name>%s</job_name>\n", job_name);
        request += string(buf);
    }
    request += "</get_templates>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("get_templates reply: %s", buf);
#endif
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (strstr(buf, "<templates>")) {
            MIOFILE mf;
            XML_PARSER xp(&mf);
            mf.init_file(reply);
            retval = td.parse(xp);
        }
    }
    fclose(reply);
    return retval;
}

int TEMPLATE_DESC::parse(XML_PARSER& xp) {
    int retval;
    string s;
    while (!xp.get_tag()) {
        if (xp.match_tag("input_template")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/input_template")) break;
                if (xp.parse_string("open_name", s)) {
                    input_files.push_back(s);
                }
            }
        }
        if (xp.match_tag("output_template")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/output_template")) break;
                if (xp.parse_string("open_name", s)) {
                    output_files.push_back(s);
                }
            }
        }
    }
    return 0;
}

int COMPLETED_JOB_DESC::parse(XML_PARSER& xp) {
    canonical_resultid = 0;
    error_mask = 0;
    error_resultid = 0;
    exit_status = 0;
    elapsed_time = 0;
    cpu_time = 0;
    while (!xp.get_tag()) {
        if (xp.match_tag("/completed_job")) return 0;
        if (xp.parse_int("canonical_resultid", canonical_resultid)) continue;
        if (xp.parse_int("error_mask", error_mask)) continue;
        if (xp.parse_int("error_resultid", error_resultid)) continue;
        if (xp.parse_int("exit_status", exit_status)) continue;
        if (xp.parse_double("elapsed_time", elapsed_time)) continue;
        if (xp.parse_double("cpu_time", cpu_time)) continue;
        if (xp.parse_string("stderr_out", stderr_out)) {
            xml_unescape(stderr_out);
            continue;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file(
    const char* project_url,
    const char* authenticator,
    const char* job_name,
    int file_num,
    const char* dst_path,
    string &error_msg
) {
    char url[1024];
    sprintf(url, "%sget_output.php?cmd=workunit_file&auth_str=%s&wu_name=%s&file_num=%d",
        project_url, authenticator, job_name, file_num
    );
    //printf("fetching %s to %s\n", url, dst_path);
    int retval = do_http_get(url, dst_path);
    error_msg = "";
    if (retval) {
        char buf[1024];
        sprintf(buf, "couldn't fetch %s: %d", url, retval);
        error_msg = string(buf);
    }
    return retval;
}

int query_completed_job(
    const char* project_url,
    const char* authenticator,
    const char* job_name,
    COMPLETED_JOB_DESC& jd,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<query_completed_job>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    sprintf(buf, "<job_name>%s</job_name>\n", job_name);
    request += string(buf);
    request += "</query_completed_job>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("query_completed_job reply: %s", buf);
#endif
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (strstr(buf, "<completed_job>")) {
            MIOFILE mf;
            XML_PARSER xp(&mf);
            mf.init_file(reply);
            retval = jd.parse(xp);
        }
    }
    fclose(reply);
    return retval;
}

int retire_batch(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<retire_batch>\n";
    sprintf(buf, "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    sprintf(buf, "<batch_name>%s</batch_name>\n", batch_name);
    request += string(buf);
    request += "</retire_batch>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("retire_batch reply: %s", buf);
#endif
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (strstr(buf, "success")) {
            retval = 0;
            continue;
        }
    }
    fclose(reply);
    return retval;
}

int ping_server(
    const char* project_url,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<ping> </ping>\n";   // the space is needed
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
#ifdef SHOW_REPLY
        printf("reply: %s\n", buf);
#endif
        if (parse_int(buf, "<error_num>", retval)) continue;
        if (parse_str(buf, "<error_msg>", error_msg)) continue;
        if (strstr(buf, "success")) {
            retval = 0;
            continue;
        }
    }
    fclose(reply);
    return retval;
}
