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

#include <curl/curl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>

#include "parse.h"

#include "curl.h"

using std::vector;
using std::string;

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
    int batch_id,
    vector<string> &md5s,
    vector<string> &paths,
    vector<int> &absent_files
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
    while (fgets(buf, 256, reply)) {
        printf("reply: %s", buf);
        if (strstr(buf, "error")) {
            retval = -1;
        }
        if (parse_int(buf, "<absent_file>", x)) {
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
    int batch_id,
    vector<string> &md5s,
    vector<string> &paths
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
    while (fgets(buf, 256, reply)) {
        printf("reply: %s", buf);
        if (strstr(buf, "success")) {
            success = true;
            break;
        }
    }
    fclose(reply);
    if (!success) return -1;
    return 0;
}

int create_batch(
    const char* project_url,
    const char* authenticator,
    SUBMIT_REQ& sr
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
        sr.batch_name,
        sr.app_name
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
    sr.batch_id = 0;
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
        if (parse_int(buf, "<batch_id>", sr.batch_id)) break;
    }
    fclose(reply);
    if (sr.batch_id == 0) {
        return -1;
    }
    return 0;
}

int submit_jobs(
    const char* project_url,
    const char* authenticator,
    SUBMIT_REQ req
) {
    char buf[1024], url[1024];
    sprintf(buf,
        "<create_batch>\n"
        "<authenticator>%s</authenticator>\n"
        "<batch_id>%d</batch_id>\n",
        authenticator,
        req.batch_id
    );
    string request = buf;
    for (unsigned int i=0; i<req.jobs.size(); i++) {
        JOB job=req.jobs[i];
        request += "<job>\n";
        if (!job.cmdline_args.empty()) {
            request += "<command_line>" + job.cmdline_args + "</command_line>\n";
        }
        for (unsigned int j=0; j<job.infiles.size(); j++) {
            INFILE infile = job.infiles[i];
            map<string, LOCAL_FILE>::iterator iter = req.local_files.find(infile.src_path);
            LOCAL_FILE& lf = iter->second;
            sprintf(buf,
                "<input_file>\n"
                "<mode>local</mode>\n"
                "<path>%s</path>\n"
                "</input_file>\n",
                lf.md5
            );
            request += buf;
        }
        request += "</job>\n";
    }
    request += "</create_batch>\n";
    sprintf(url, "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    while (fgets(buf, 256, reply)) {
    }
    fclose(reply);
    return 0;
}
