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
// https://github.com/BOINC/boinc/wiki/RemoteInputFiles
// https://github.com/BOINC/boinc/wiki/RemoteOutputFiles
// https://github.com/BOINC/boinc/wiki/RemoteJobs

#include <curl/curl.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <string.h>

#include "parse.h"
#include "str_util.h"
#include "url.h"

#include "remote_submit.h"

using std::vector;
using std::string;

//#define SHOW_REQUEST
//#define SHOW_REPLY

// replies can have one or more <error> elements.
// These can be either PHP Notices or Warnings (which are not fatal),
// or fatal errors.
// Fatal errors have nonzero error_num.
//
struct RS_ERROR {
    int error_num;
    char error_msg[256];
    char type[256];
    char file[256];
    char line[256];

    void parse(XML_PARSER& xp) {
        error_num = 0;
        strcpy(error_msg, "");
        strcpy(type, "");
        strcpy(file, "");
        strcpy(line, "");
        while (!xp.get_tag()) {
            if (xp.match_tag("/error")) break;
            if (xp.parse_str("error_msg", error_msg, sizeof(error_msg))) continue;
            if (xp.parse_int("error_num", error_num)) continue;
            if (xp.parse_str("type", type, sizeof(type))) continue;
            if (xp.parse_str("file", file, sizeof(file))) continue;
            if (xp.parse_str("line", line, sizeof(line))) continue;
        }
    }
};

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

#ifdef SHOW_REQUEST
    printf("HTTP request:\n%s\n", request);
#endif

    struct curl_httppost *formpost=NULL;
    struct curl_httppost *lastptr=NULL;
    struct curl_slist *headerlist=NULL;

    curl_formadd(&formpost, &lastptr,
        CURLFORM_COPYNAME, "request",
        CURLFORM_COPYCONTENTS, request,
        CURLFORM_END
    );
    for (unsigned int i=0; i<send_files.size(); i++) {
        snprintf(buf, sizeof(buf), "file_%d", i);
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
    vector<string> &boinc_names,
    int batch_id,
    vector<int> &absent_files,
    string& error_msg
) {
    string req_msg;
    char buf[256];
    req_msg = "<query_files>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    req_msg += string(buf);
    if (batch_id) {
        snprintf(buf, sizeof(buf), "<batch_id>%d</batch_id>\n", batch_id);
        req_msg += string(buf);
    }
    for (unsigned int i=0; i<boinc_names.size(); i++) {
        snprintf(buf, sizeof(buf), "   <phys_name>%s</phys_name>\n", boinc_names[i].c_str());
        req_msg += string(buf);
    }
    req_msg += "</query_files>\n";
    FILE* reply = tmpfile();
    char url[256];
    snprintf(url, sizeof(url), "%sjob_file.php", project_url);
    vector<string> xx;
    int retval = do_http_post(url, req_msg.c_str(), reply, xx);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    int x;
    retval = 0;
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("query_files reply: %s\n", xp.parsed_tag);
#endif
        if (xp.match_tag("absent_files")) {
            while (!xp.get_tag()) {
                if (xp.match_tag("/absent_files")) break;
                if (xp.parse_int("file", x)) {
                    absent_files.push_back(x);
                }
            }
        } else if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int upload_files (
    const char* project_url,
    const char* authenticator,
    vector<string> &paths,
    vector<string> &boinc_names,
    int batch_id,
    string &error_msg
) {
    char buf[1024];
    string req_msg = "<upload_files>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    req_msg += string(buf);
    if (batch_id) {
        snprintf(buf, sizeof(buf), "<batch_id>%d</batch_id>\n", batch_id);
        req_msg += string(buf);
    }
    for (unsigned int i=0; i<boinc_names.size(); i++) {
        snprintf(buf, sizeof(buf), "<phys_name>%s</phys_name>\n", boinc_names[i].c_str());
        req_msg += string(buf);
    }
    req_msg += "</upload_files>\n";
    FILE* reply = tmpfile();
    char url[256];
    snprintf(url, sizeof(url), "%sjob_file.php", project_url);
    int retval = do_http_post(url, req_msg.c_str(), reply, paths);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    bool success;
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("upload_files reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_bool("success", success)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int create_batch(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    const char* app_name,
    double expire_time,
    int& batch_id,
    string& error_msg
) {
    char request[1024];
    char url[1024];
    snprintf(request, sizeof(request),
        "<create_batch>\n"
        "   <authenticator>%s</authenticator>\n"
        "   <batch_name>%s</batch_name>\n"
        "   <app_name>%s</app_name>\n"
        "   <expire_time>%f</expire_time>\n"
        "</create_batch>\n",
        authenticator,
        batch_name,
        app_name,
        expire_time
    );
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request, reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    batch_id = 0;
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    retval = 0;
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("create_batch reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_int("batch_id", batch_id)) continue;
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int estimate_batch(
    const char* project_url,
    const char* authenticator,
    char app_name[256],
    vector<JOB> jobs,
    double& est_makespan,
    string& error_msg
) {
    char buf[1024], url[1024];
    snprintf(buf, sizeof(buf),
        "<estimate_batch>\n"
        "<authenticator>%s</authenticator>\n"
        "<batch>\n"
        "   <app_name>%s</app_name>\n",
        authenticator,
        app_name
    );
    string request = buf;
    for (unsigned int i=0; i<jobs.size(); i++) {
        JOB job = jobs[i];
        request += "<job>\n";
        if (!job.cmdline_args.empty()) {
            request += "<command_line>" + job.cmdline_args + "</command_line>\n";
        }
        request += "</job>\n";
    }
    request += "</batch>\n</estimate_batch>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("estimate_batch reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_double("seconds", est_makespan)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int submit_jobs(
    const char* project_url,
    const char* authenticator,
    char app_name[256],
    int batch_id,
    vector<JOB> jobs,
    string& error_msg,
    int app_version_num
) {
    JOB_PARAMS jp;
    return submit_jobs_params(
        project_url,
        authenticator,
        app_name,
        batch_id,
        jobs,
        error_msg,
        jp,
        app_version_num
    );
}

int submit_jobs_params(
    const char* project_url,
    const char* authenticator,
    char app_name[256],
    int batch_id,
    vector<JOB> jobs,
    string& error_msg,
    JOB_PARAMS &job_params,
    int app_version_num
) {
    char buf[1024], url[1024];
    snprintf(buf, sizeof(buf),
        "<submit_batch>\n"
        "<authenticator>%s</authenticator>\n"
        "<batch>\n"
        "   <batch_id>%d</batch_id>\n"
        "   <app_name>%s</app_name>\n"
        "   <app_version_num>%d</app_version_num>\n"
        "   <job_params>\n"
        "       <rsc_disk_bound>%f</rsc_disk_bound>\n"
        "       <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "       <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "       <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "       <delay_bound>%f</delay_bound>\n"
        "   </job_params>\n",
        authenticator,
        batch_id,
        app_name,
        app_version_num,
        job_params.rsc_disk_bound,
        job_params.rsc_fpops_est,
        job_params.rsc_fpops_bound,
        job_params.rsc_memory_bound,
        job_params.delay_bound
    );
    string request = buf;
    for (unsigned int i=0; i<jobs.size(); i++) {
        JOB job = jobs[i];
        request += "<job>\n";
        snprintf(buf, sizeof(buf), "  <name>%s</name>\n", job.job_name);
        request += buf;
        if (!job.cmdline_args.empty()) {
            request += "<command_line>" + job.cmdline_args + "</command_line>\n";
        }
        for (unsigned int j=0; j<job.infiles.size(); j++) {
            INFILE infile = job.infiles[j];
            switch (infile.mode) {
            case FILE_MODE_LOCAL_STAGED:
                snprintf(buf, sizeof(buf),
                    "<input_file>\n"
                    "<mode>local_staged</mode>\n"
                    "<source>%s</source>\n"
                    "</input_file>\n",
                    infile.physical_name
                );
                break;
            case FILE_MODE_REMOTE:
                snprintf(buf, sizeof(buf),
                    "<input_file>\n"
                    "<mode>remote</mode>\n"
                    "<url>%s</url>\n"
                    "<nbytes>%f</nbytes>\n"
                    "<md5>%s</md5>\n"
                    "</input_file>\n",
                    infile.url,
                    infile.nbytes,
                    infile.md5
                );
                break;
            default:
                fprintf(stderr, "unsupported file mode %d\n", infile.mode);
                exit(1);
            }
            request += buf;
        }
        request += "</job>\n";
    }
    request += "</batch>\n</submit_batch>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    int temp;
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("submit_batch reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_int("batch_id", temp)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int query_batch_set(
    const char* project_url,
    const char* authenticator,
    double min_mod_time,
    vector<string> &batch_names,
    QUERY_BATCH_SET_REPLY& qb_reply,
    string& error_msg
) {
    string request;
    char url[1024], buf[256];
    int batch_size;

    request = "<query_batch2>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    snprintf(buf, sizeof(buf), "<min_mod_time>%f</min_mod_time>\n", min_mod_time);
    request += string(buf);
    for (unsigned int i=0; i<batch_names.size(); i++) {
        snprintf(buf, sizeof(buf), "<batch_name>%s</batch_name>\n", batch_names[i].c_str());
        request += string(buf);
    }
    request += "</query_batch2>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    qb_reply.server_time = 0;
    MIOFILE mf;
    mf.init_file(reply);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("query_batches reply: %s", buf);
#endif
        if (xp.match_tag("query_batch2")) {
            retval = 0;
            continue;
        }
        if (xp.parse_double("server_time", qb_reply.server_time)) continue;
        if (xp.parse_int("batch_size", batch_size)) {
            qb_reply.batch_sizes.push_back(batch_size);
            continue;
        }
        if (xp.match_tag("job")) {
            JOB_STATUS js;
            while (!xp.get_tag()) {
#ifdef SHOW_REPLY
                printf("query_batches reply: %s", buf);
#endif
                if (xp.match_tag("/job")) {
                    qb_reply.jobs.push_back(js);
                    break;
                }
                if (xp.parse_string("job_name", js.job_name)) continue;
                if (xp.parse_string("status", js.status)) continue;
            }
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int BATCH_STATUS::parse(XML_PARSER& xp) {
    static const BATCH_STATUS x;
    *this = x;
    while (!xp.get_tag()) {
        if (xp.match_tag("/batch")) {
            return 0;
        }
        if (xp.parse_int("id", id)) continue;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_int("state", state)) continue;
        if (xp.parse_int("njobs", njobs)) continue;
        if (xp.parse_int("nerror_jobs", nerror_jobs)) continue;
        if (xp.parse_double("fraction_done", fraction_done)) continue;
        if (xp.parse_double("create_time", create_time)) continue;
        if (xp.parse_double("expire_time", expire_time)) continue;
        if (xp.parse_double("est_completion_time", est_completion_time)) continue;
        if (xp.parse_double("completion_time", completion_time)) continue;
        if (xp.parse_double("credit_estimate", credit_estimate)) continue;
        if (xp.parse_double("credit_canonical", credit_canonical)) continue;
    }
    return ERR_XML_PARSE;
}

void BATCH_STATUS::print() {
    printf("Batch %d (%s)\n"
        "   state: %s\n"
        "   njobs: %d\n"
        "   nerror_jobs: %d\n"
        "   fraction_done: %f\n",
        id, name,
        batch_state_string(state),
        njobs,
        nerror_jobs,
        fraction_done
    );
    printf(
        "   create_time: %s\n",
        time_to_string(create_time)
    );
    printf(
        "   expire_time: %s\n",
        time_to_string(expire_time)
    );
    printf(
        "   est_completion_time: %s\n",
        time_to_string(est_completion_time)
    );
    printf(
        "   completion_time: %s\n",
        time_to_string(completion_time)
    );
    printf(
        "   credit_estimate: %f\n"
        "   credit_canonical: %f\n",
        credit_estimate,
        credit_canonical
    );
}

int query_batches(
    const char* project_url,
    const char* authenticator,
    vector<BATCH_STATUS>& batches,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<query_batches>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    request += "</query_batches>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    error_msg = "failed to parse Web RPC reply";
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
        if (xp.match_tag("/query_batches")) {
            retval = 0;
            error_msg = "";
            break;
        }
        if (xp.match_tag("batch")) {
            BATCH_STATUS bs;
            if (!bs.parse(xp)) {
                batches.push_back(bs);
            }
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int JOB_STATE::parse(XML_PARSER& xp) {
    static const JOB_STATE x;
    *this = x;
    while (!xp.get_tag()) {
        if (xp.match_tag("/job")) {
            return 0;
        }
        if (xp.parse_int("id", id)) continue;
        if (xp.parse_str("name", name, sizeof(name))) continue;
        if (xp.parse_int("canonical_instance_id", canonical_instance_id)) continue;
        if (xp.parse_int("n_outfiles", n_outfiles)) continue;
    }
    return ERR_XML_PARSE;
}

void JOB_STATE::print() {
    printf(
        "job %d (%s)\n"
        "   canonical_instance_id %d\n"
        "   n_outfiles %d\n",
        id, name,
        canonical_instance_id,
        n_outfiles
    );
}

int query_batch(
    const char* project_url,
    const char* authenticator,
    int batch_id,
    const char* batch_name,
    vector<JOB_STATE>& jobs,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<query_batch>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    if (batch_id) {
        snprintf(buf, sizeof(buf), "<batch_id>%d</batch_id>\n", batch_id);
    } else {
        snprintf(buf, sizeof(buf), "<batch_name>%s</batch_name>\n", batch_name);
    }
    request += string(buf);
    request += "</query_batch>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
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
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
        if (xp.match_tag("/jobs")) {
            retval = 0;
            break;
        }
        if (xp.match_tag("job")) {
            JOB_STATE js;
            if (!js.parse(xp)) {
                jobs.push_back(js);
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
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    for (unsigned int i=0; i<job_names.size(); i++) {
        snprintf(buf, sizeof(buf), "<job_name>%s</job_name>\n", job_names[i].c_str());
        request += string(buf);
    }
    request += "</abort_jobs>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    fseek(reply, 0, SEEK_SET);
    retval = -1;
    bool success;
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("abort_jobs reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_bool("success", success)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
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
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    if (app_name) {
        snprintf(buf, sizeof(buf), "<app_name>%s</app_name>\n", app_name);
        request += string(buf);
    } else {
        snprintf(buf, sizeof(buf), "<job_name>%s</job_name>\n", job_name);
        request += string(buf);
    }
    request += "</get_templates>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("get_templates reply: %s\n", xp.parsed_tag);
#endif
        if (xp.match_tag("templates")) {
            retval = td.parse(xp);
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int TEMPLATE_DESC::parse(XML_PARSER& xp) {
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
    char url[1024], job_name_esc[1024];
    escape_url(job_name, job_name_esc, sizeof(job_name_esc));
    snprintf(url, sizeof(url), "%sget_output.php?cmd=workunit_file&auth_str=%s&wu_name=%s&file_num=%d",
        project_url, authenticator, job_name_esc, file_num
    );
    //printf("fetching %s to %s\n", url, dst_path);
    int retval = do_http_get(url, dst_path);
    error_msg = "";
    if (retval) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "couldn't fetch %s: %d", url, retval);
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
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    snprintf(buf, sizeof(buf), "<job_name>%s</job_name>\n", job_name);
    request += string(buf);
    request += "</query_completed_job>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("query_completed_job reply: %s\n", xp.parsed_tag);
#endif
        if (xp.match_tag("completed_job")) {
            retval = jd.parse(xp);
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
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
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    snprintf(buf, sizeof(buf), "<batch_name>%s</batch_name>\n", batch_name);
    request += string(buf);
    request += "</retire_batch>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    bool success;
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("retire_batch reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_bool("success", success)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}

int set_expire_time(
    const char* project_url,
    const char* authenticator,
    const char* batch_name,
    double expire_time,
    string &error_msg
) {
    string request;
    char url[1024], buf[256];
    request = "<set_expire_time>\n";
    snprintf(buf, sizeof(buf), "<authenticator>%s</authenticator>\n", authenticator);
    request += string(buf);
    snprintf(buf, sizeof(buf), "<batch_name>%s</batch_name>\n", batch_name);
    request += string(buf);
    snprintf(buf, sizeof(buf), "<expire_time>%f</expire_time>\n", expire_time);
    request += string(buf);
    request += "</set_expire_time>\n";
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    bool success;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("set_expire_time reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_bool("success", success)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
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
    char url[1024];
    request = "<ping> </ping>\n";   // the space is needed
    snprintf(url, sizeof(url), "%ssubmit_rpc_handler.php", project_url);
    FILE* reply = tmpfile();
    vector<string> x;
    int retval = do_http_post(url, request.c_str(), reply, x);
    if (retval) {
        fclose(reply);
        return retval;
    }
    retval = -1;
    bool success;
    error_msg = "";
    fseek(reply, 0, SEEK_SET);
    MIOFILE mf;
    XML_PARSER xp(&mf);
    mf.init_file(reply);
    while (!xp.get_tag()) {
#ifdef SHOW_REPLY
        printf("ping_server() reply: %s\n", xp.parsed_tag);
#endif
        if (xp.parse_bool("success", success)) {
            retval = 0;
            continue;
        }
        if (xp.match_tag("error")) {
            RS_ERROR error;
            error.parse(xp);
            if (error.error_num) {
                retval = error.error_num;
                error_msg = error.error_msg;
            }
        }
    }
    fclose(reply);
    return retval;
}
