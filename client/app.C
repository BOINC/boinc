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

// Abstraction of a set of executing applications,
// connected to I/O files in various ways.
// Shouldn't depend on CLIENT_STATE.

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>

#include "types.h"
#include "client_state.h"
#include "filesys.h"
#include "file_names.h"
#include "log_flags.h"
#include "parse.h"

#include "app.h"

// take a string containing some words.
// return an array of pointers to the null-terminated words.
// Modifies the string arg.
//
void parse_command_line(char* p, char** argv) {
    char** pp = argv;
    bool space = true;

    while (*p) {
        if (isspace(*p)) {
            *p = 0;
            space = true;
        } else {
            if (space) {
                *pp++ = p;
                space = false;
            }
        }
        p++;
    }
    *pp++ = 0;
}

static void print_argv(char** argv) {
    int i;
    for (i=0; argv[i]; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
}

ACTIVE_TASK::ACTIVE_TASK() {
    result = NULL;
    wup = NULL;
    app_version = NULL;
    slot = 0;
    exit_status = 0;
    signal = 0;
    strcpy(dirname, "");
    cpu_time = 0;
}

int ACTIVE_TASK::init(RESULT* rp) {
    result = rp;
    wup = rp->wup;
    app_version = wup->avp;
    return 0;
}

int ACTIVE_TASK::start(bool first_time) {
    char exec_name[256], file_path[256], link_path[256];
    char* argv[100];
    unsigned int i;
    IO_FILE_DESC ifd;
    int fd, retval;


#ifdef unix
    pid = fork();
    if (pid == 0) {
        
        // from here on we're running in a new process.
        // If an error happens, exit nonzero so that the core client
        // knows there was a problem.

        // make a link to the executable
        get_pathname(app_version->file_info, file_path);
        strcpy(exec_name, app_version->file_info->name);
        if (first_time) {
            sprintf(link_path, "%s/%s", dirname, exec_name);
            retval = link(file_path, link_path);
            if (log_flags.task_debug) {
                printf("link %s to %s\n", file_path, link_path);
            }
            if (retval) {
                perror("link");
                exit(retval);
            }
        }
        
        // create symbolic links, and hook up descriptors, for input files
        //
        for (i=0; i<wup->input_files.size(); i++) {
            ifd = wup->input_files[i];
            get_pathname(ifd.file_info, file_path);
            if (strlen(ifd.open_name)) {
                if (first_time) {
                    sprintf(link_path, "%s/%s", dirname, ifd.open_name);
                    if (log_flags.task_debug) {
                        printf("link %s to %s\n", file_path, link_path);
                    }
                    retval = link(file_path, link_path);
                    if (retval) {
                        perror("link");
                        exit(retval);
                    }
                }
            } else {
                fd = open(file_path, O_RDONLY);
                if (fd != ifd.fd) {
                    retval = dup2(fd, ifd.fd);
                    if (retval < 0) {
                        fprintf(stderr, "dup2 %d %d returned %d\n", fd, ifd.fd, retval);
                        exit(retval);
                    }
                    close(fd);
                }
            }
        }

        // hook up the output files
        //
        for (i=0; i<result->output_files.size(); i++) {
            ifd = result->output_files[i];
            get_pathname(ifd.file_info, file_path);
            if (strlen(ifd.open_name)) {
                if (first_time) {
                    creat(file_path, 0660);
                    sprintf(link_path, "%s/%s", dirname, ifd.open_name);
                    if (log_flags.task_debug) {
                        printf("link %s to %s\n", file_path, link_path);
                    }
                    retval = link(file_path, link_path);
                    if (retval) {
                        perror("link");
                        exit(retval);
                    }
                }
            } else {
                fd = open(file_path, O_WRONLY|O_CREAT, 0660);
                if (fd != ifd.fd) {
                    retval = dup2(fd, ifd.fd);
                    if (retval < 0) {
                        fprintf(stderr, "dup2 %d %d returned %d\n", fd, ifd.fd, retval);
                        exit(retval);
                    }
                    close(fd);
                }
            }
        }

        // chdir() into the slot directory
        //
        retval = chdir(dirname);
        if (retval) {
            perror("chdir");
            exit(retval);
        }

        // hook up stderr to a specially-named file
        //
        freopen(STDERR_FILE, "a", stderr);

        argv[0] = exec_name;
        parse_command_line(wup->command_line, argv+1);
        if (log_flags.task_debug) print_argv(argv);
        retval = execv(exec_name, argv);
        fprintf(stderr, "execv failed: %d\n", retval);
        perror("execv");
        exit(1);
    }
    if (log_flags.task_debug) printf("forked process: pid %d\n", pid);
#endif

#ifdef _WIN32
#endif

#ifdef macintosh
#endif

    state = PROCESS_RUNNING;
    return 0;
}

int ACTIVE_TASK_SET::insert(ACTIVE_TASK* atp) {
    int retval;

    get_slot_dir(atp->slot, atp->dirname);
    clean_out_dir(atp->dirname);
    retval = atp->start(true);
    if (retval) return retval;
    active_tasks.push_back(atp);
    return 0;
}

// check for child process exit
//
bool ACTIVE_TASK_SET::poll() {
    int pid;
    int stat;
    ACTIVE_TASK* atp;
    struct rusage rs;
    char path[256];
    int n;

#ifdef unix
    pid = wait3(&stat, WNOHANG, &rs);
    if (pid <= 0) return false;
    if (log_flags.task_debug) printf("got signal for process %d\n", pid);
    atp = lookup_pid(pid);
    if (!atp) {
        fprintf(stderr, "ACTIVE_TASK_SET::poll(): pid %d not found\n", pid);
        return true;
    }
    atp->result->cpu_time = rs.ru_utime.tv_sec + rs.ru_utime.tv_usec/1.e6;
    if (WIFEXITED(stat)) {
        atp->state = PROCESS_EXITED;
        atp->exit_status = WEXITSTATUS(stat);
        atp->result->exit_status = atp->exit_status;
    } else if (WIFSIGNALED(stat)) {
        atp->state = PROCESS_WAS_SIGNALED;
        atp->signal = WTERMSIG(stat);
        atp->result->exit_status = atp->signal;
    } else {
        atp->state = PROCESS_EXIT_UNKNOWN;
        atp->result->exit_status = -1;
    }

    // check for the stderr file, copy to result record
    //
    sprintf(path, "%s/%s", atp->dirname, STDERR_FILE);
    FILE* f = fopen(path, "r");
    if (f) {
        n = fread(atp->result->stderr_out, 1, STDERR_MAX_LEN, f);
        atp->result->stderr_out[STDERR_MAX_LEN-1] = 0;
        fclose(f);
    }

    clean_out_dir(atp->dirname);
#endif

    return true;
}

ACTIVE_TASK* ACTIVE_TASK_SET::lookup_pid(int pid) {
    unsigned int i;
    ACTIVE_TASK* atp;

    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        if (atp->pid == pid) return atp;
    }
    return 0;
}

void ACTIVE_TASK_SET::suspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        kill(atp->pid, SIGSTOP);
    }
}

void ACTIVE_TASK_SET::unsuspend_all() {
    unsigned int i;
    ACTIVE_TASK* atp;
    for (i=0; i<active_tasks.size(); i++) {
        atp = active_tasks[i];
        kill(atp->pid, SIGCONT);
    }
}

int ACTIVE_TASK_SET::remove(ACTIVE_TASK* atp) {
    vector<ACTIVE_TASK*>::iterator iter;

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        if (*iter == atp) {
            active_tasks.erase(iter);
            return 0;
        }
        iter++;
    }
    fprintf(stderr, "ACTIVE_TASK_SET::remove(): not found\n");
    return 1;
}

int ACTIVE_TASK_SET::restart_tasks() {
    vector<ACTIVE_TASK*>::iterator iter;
    ACTIVE_TASK* atp;
    int retval;

    iter = active_tasks.begin();
    while (iter != active_tasks.end()) {
        atp = *iter;
        get_slot_dir(atp->slot, atp->dirname);
        retval = atp->start(false);
        if (log_flags.task) {
            printf("restarting application for result %s\n", atp->result->name);
        }
        if (retval) {
            fprintf(stderr, "ACTIVE_TASKS::restart_tasks(); restart failed: %d\n", retval);
            active_tasks.erase(iter);
        } else {
            iter++;
        }
    }
    return 0;
}

int ACTIVE_TASK::write(FILE* fout) {
    fprintf(fout,
        "<active_task>\n"
        "    <project_domain>%s</project_domain>\n"
        "    <result_name>%s</result_name>\n"
        "    <app_version_num>%d</app_version_num>\n"
        "    <slot>%d</slot>\n"
        "    <cpu_time>%f</cpu_time>\n"
        "</active_task>\n",
        result->project->domain,
        result->name,
        app_version->version_num,
        slot,
        cpu_time
    );
    return 0;
}

int ACTIVE_TASK::parse(FILE* fin, CLIENT_STATE* cs) {
    char buf[256], result_name[256], project_domain[256];
    int app_version_num=0;
    PROJECT* project;

    strcpy(result_name, "");
    strcpy(project_domain, "");
    cpu_time = 0;
    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</active_task>")) {
            project = cs->lookup_project(project_domain);
            if (!project) {
                fprintf(stderr,
                    "ACTIVE_TASK::parse(): project not found: %s\n",
                    project_domain
                );
                return -1;
            }
            result = cs->lookup_result(project, result_name);
            if (!result) {
                fprintf(stderr, "ACTIVE_TASK::parse(): result not found\n");
                return -1;
            }
            wup = result->wup;
            app_version = cs->lookup_app_version(
                result->app, app_version_num
            );
            if (!app_version) {
                fprintf(stderr, "ACTIVE_TASK::parse(): app_version not found\n");
                return -1;
            }
            return 0;
        }
        else if (parse_str(buf, "<result_name>", result_name)) continue;
        else if (parse_str(buf, "<project_domain>", project_domain)) continue;
        else if (parse_int(buf, "<app_version_num>", app_version_num)) continue;
        else if (parse_int(buf, "<slot>", slot)) continue;
        else if (parse_double(buf, "<cpu_time>", cpu_time)) continue;
        else fprintf(stderr, "ACTIVE_TASK::parse(): unrecognized %s\n", buf);
    }
    return -1;
}

int ACTIVE_TASK_SET::write(FILE* fout) {
    unsigned int i;
    fprintf(fout, "<active_task_set>\n");
    for (i=0; i<active_tasks.size(); i++) {
        active_tasks[i]->write(fout);
    }
    fprintf(fout, "</active_task_set>\n");
    return 0;
}

int ACTIVE_TASK_SET::parse(FILE* fin, CLIENT_STATE* cs) {
    ACTIVE_TASK* atp;
    char buf[256];
    int retval;

    while (fgets(buf, 256, fin)) {
        if (match_tag(buf, "</active_task_set>")) return 0;
        else if (match_tag(buf, "<active_task>")) {
            atp = new ACTIVE_TASK;
            retval = atp->parse(fin, cs);
            if (!retval) active_tasks.push_back(atp);
            else delete atp;
        } else {
            fprintf(stderr, "ACTIVE_TASK_SET::parse(): unrecognized %s\n", buf);
        }
    }
    return 0;
}
