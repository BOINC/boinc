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

#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"
#include "parse.h"
#include "client_state.h"

// Scan project dir for file names of the form trickle_up_X_Y
// where X is a result name and Y is a timestamp.
// Convert them to XML (for sched request message)
//
int CLIENT_STATE::read_trickle_files(PROJECT* project, FILE* f) {
    char project_dir[256], *p, *q, result_name[256], fname[256];
    char* file_contents, path[256];
    string fn;
    time_t t;
    int retval;

    get_project_dir(project, project_dir);
    DirScanner ds(project_dir);

    while (ds.scan(fn)) {
        strcpy(fname, fn.c_str());
        if (strstr(fname, "trickle_up_") != fname) continue;
        q = fname + strlen("trickle_up_");
        p = strrchr(fname, '_');
        if (p <= q) continue;
        *p = 0;
        strcpy(result_name, q);
        *p = '_';
        t = atoi(p+1);

        sprintf(path, "%s%s%s", project_dir, PATH_SEPARATOR, fname);
        retval = read_file_malloc(path, file_contents);
        if (retval) continue;
        fprintf(f,
            "  <trickle_up>\n"
            "      <result_name>%s</result_name>\n"
            "      <time>%d</time>\n"
            "      <text>\n"
            "%s\n"
            "      </text>\n"
            "  </trickle_up>\n",
            result_name,
            (int)t,
            file_contents
        );
        free(file_contents);
    }
    return 0;
}

// Remove files when ack has been received
//
int CLIENT_STATE::remove_trickle_files(PROJECT* project) {
    char project_dir[256], path[256], fname[256];
    string fn;

    get_project_dir(project, project_dir);
    DirScanner ds(project_dir);

    while (ds.scan(fn)) {
        strcpy(fname, fn.c_str());
        if (strstr(fname, "trickle_up") != fname) continue;
        sprintf(path, "%s%s%s", project_dir, PATH_SEPARATOR, fname);
        boinc_delete_file(path);
    }
    return 0;
}

// parse a trickle-down message in a scheduler reply.
// Locate the corresponding active task,
// write a file in the slot directory,
// and notify the task
//
int CLIENT_STATE::handle_trickle_down(PROJECT* project, FILE* in) {
    char buf[256];
    char result_name[256], path[256];
    string body;
    int retval, send_time;

    strcpy(result_name, "");
    while (fgets(buf, 256, in)) {
        if (match_tag(buf, "</trickle_down>")) {
            RESULT* rp = lookup_result(project, result_name);
            if (!rp) return ERR_NULL;
            ACTIVE_TASK* atp = lookup_active_task_by_result(rp);
            if (!atp) return ERR_NULL;
            sprintf(path, "%s%strickle_down_%d", atp->slot_dir, PATH_SEPARATOR, send_time);
            FILE* f = fopen(path, "w");
            if (!f) return ERR_FOPEN;
            fputs(body.c_str(), f);
            fclose(f);
            return 0;
        } else if (match_tag(buf, "<body>")) {
            retval = copy_element_contents(in, "</body>", body);
            if (retval) return retval;
        } else if (parse_str(buf, "<result_name>", result_name, 256)) {
            continue;
        } else if (parse_int(buf, "<send_time>", send_time)) {
            continue;
        } else {
            ;
        }
    }
    return ERR_XML_PARSE;
}
