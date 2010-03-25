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

// boinccmd: command-line interface to a BOINC core client,
// using GUI RPCs.
//
// usage: boinccmd [--host hostname] [--passwd passwd] command

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "win_util.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#endif

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "gui_rpc_client.h"
#include "error_numbers.h"
#include "util.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "version.h"
#include "common_defs.h"

void version(){
    printf("boinccmd,  built from %s \n", PACKAGE_STRING );
    exit(0);
}

void usage() {
    fprintf(stderr, "\n\
usage: boinccmd [--host hostname] [--passwd passwd] command\n\n\
Commands:\n\
 --lookup_account URL email passwd\n\
 --create_account URL email passwd name\n\
 --project_attach URL auth          attach to project\n\
 --join_acct_mgr URL name passwd    attach account manager\n\
 --quit_acct_mgr                    quit current account manager\n\
 --get_state                        show entire state\n\
 --get_tasks                        show tasks\n\
 --get_simple_gui_info              show status of projects and active tasks\n\
 --get_file_transfers               show file transfers\n\
 --get_project_status               show status of all attached projects\n\
 --get_disk_usage                   show disk usage\n\
 --get_proxy_settings\n\
 --get_messages [ seqno ]           show messages > seqno\n\
 --get_message_count                show largest message seqno\n\
 --get_notices [ seqno ]            show notices > seqno\n\
 --get_host_info\n\
 --version, -V                      show core client version\n\
 --task url task_name op            task operation\n\
   op = suspend | resume | abort | graphics_window | graphics_fullscreen\n\
 --project URL op                   project operation\n\
   op = reset | detach | update | suspend | resume | nomorework | allowmorework\n\
 --file_transfer URL filename op    file transfer operation\n\
   op = retry | abort\n\
 --set_run_mode mode duration       set run mode for given duration\n\
   mode = always | auto | never\n\
 --set_gpu_mode mode duration       set GPU run mode for given duration\n\
   mode = always | auto | never\n\
 --set_network_mode mode duration\n\
 --set_proxy_settings\n\
 --run_benchmarks\n\
 --read_global_prefs_override\n\
 --quit\n\
 --read_cc_config\n\
 --set_debts URL1 std1 ltd1 [URL2 std2 ltd2 ...]\n\
 --get_project_config URL\n\
 --get_project_config_poll\n\
 --network_available\n\
 --get_cc_status\n\
"
);
    exit(1);
}

void parse_display_args(char** argv, int& i, DISPLAY_INFO& di) {
    strcpy(di.window_station, "winsta0");
    strcpy(di.desktop, "default");
    strcpy(di.display, "");
    while (argv[i]) {
        if (!strcmp(argv[i], "--window_station")) {
            strlcpy(di.window_station, argv[++i], sizeof(di.window_station));
        } else if (!strcpy(argv[i], "--desktop")) {
            strlcpy(di.desktop, argv[++i], sizeof(di.desktop));
        } else if (!strcpy(argv[i], "--display")) {
            strlcpy(di.display, argv[++i], sizeof(di.display));
        }
        i++;
    }
}

void show_error(int retval) {
    fprintf(stderr, "Error %d: %s\n", retval, boincerror(retval));
}

char* next_arg(int argc, char** argv, int& i) {
    if (i >= argc) {
        fprintf(stderr, "Missing command-line argument\n");
        usage();
        exit(1);
    }
    return argv[i++];
}

const char* prio_name(int prio) {
    switch (prio) {
    case MSG_INFO: return "low";
    case MSG_USER_ALERT: return "medium";
    case MSG_INTERNAL_ERROR: return "high";
    }
    return "unknown";
}

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    int i, retval, port=0;
    MESSAGES messages;
    NOTICES notices;
	char passwd_buf[256], hostname_buf[256], *hostname=0;
    char* passwd = passwd_buf, *p;

#ifdef _WIN32
    chdir_to_data_dir();
#endif
	strcpy(passwd_buf, "");
	read_gui_rpc_password(passwd_buf);

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(stderr, "WinsockInitialize: %d\n", retval);
        exit(1);
    }
#endif
    if (argc < 2) usage();
    i = 1;
    if (!strcmp(argv[i], "--help")) usage();
    if (!strcmp(argv[i], "-h"))     usage();
    if (!strcmp(argv[i], "--version")) version();
    if (!strcmp(argv[i], "-V"))     version();

    if (!strcmp(argv[i], "--host")) {
        if (++i == argc) usage();
        strlcpy(hostname_buf, argv[i], sizeof(hostname_buf));
        hostname = hostname_buf;
        p = strchr(hostname, ':');
        if (p) {
            port = atoi(p+1);
            *p=0;
        }
        i++;
    }
    if ((i<argc)&& !strcmp(argv[i], "--passwd")) {
        if (++i == argc) usage();
        passwd = argv[i];
        i++;
    }

    // change the following to debug GUI RPC's asynchronous connection mechanism
    //
#if 1
    retval = rpc.init(hostname, port);
    if (retval) {
        fprintf(stderr, "can't connect to %s\n", strlen(hostname)?hostname:"local host");
        exit(1);
    }
#else
    retval = rpc.init_asynch(hostname, 60., false);
    while (1) {
        retval = rpc.init_poll();
        if (!retval) break;
        if (retval == ERR_RETRY) {
            printf("sleeping\n");
            sleep(1);
            continue;
        }
        fprintf(stderr, "can't connect: %d\n", retval);
        exit(1);
    }
    printf("connected\n");
#endif

    if (passwd) {
        retval = rpc.authorize(passwd);
        if (retval) {
            fprintf(stderr, "Authorization failure: %d\n", retval);
            exit(1);
        }
    }

    char* cmd = next_arg(argc, argv, i);
    if (!strcmp(cmd, "--get_state")) {
        CC_STATE state;
        retval = rpc.get_state(state);
        if (!retval) state.print();
    } else if (!strcmp(cmd, "--get_tasks")) {
        RESULTS results;
        retval = rpc.get_results(results);
        if (!retval) results.print();
    } else if (!strcmp(cmd, "--get_file_transfers")) {
        FILE_TRANSFERS ft;
        retval = rpc.get_file_transfers(ft);
        if (!retval) ft.print();
    } else if (!strcmp(cmd, "--get_project_status")) {
        PROJECTS ps;
        retval = rpc.get_project_status(ps);
        if (!retval) ps.print();
    } else if (!strcmp(cmd, "--get_simple_gui_info")) {
        SIMPLE_GUI_INFO info;
        retval = rpc.get_simple_gui_info(info);
        if (!retval) info.print();
    } else if (!strcmp(cmd, "--get_disk_usage")) {
        DISK_USAGE du;
        retval = rpc.get_disk_usage(du);
        if (!retval) du.print();
    } else if (!strcmp(cmd, "--task")) {
        RESULT result;
        char* project_url = next_arg(argc, argv, i);
        strcpy(result.project_url, project_url);
        char* name = next_arg(argc, argv, i);
        strcpy(result.name, name);
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "suspend")) {
            retval = rpc.result_op(result, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.result_op(result, "resume");
        } else if (!strcmp(op, "abort")) {
            retval = rpc.result_op(result, "abort");
        } else if (!strcmp(op, "graphics_window")) {
            DISPLAY_INFO di;
            parse_display_args(argv, i, di);
            retval = rpc.show_graphics(project_url, name, MODE_WINDOW, di);
        } else if (!strcmp(op, "graphics_fullscreen")) {
            DISPLAY_INFO di;
            parse_display_args(argv, i, di);
            retval = rpc.show_graphics(project_url, name, MODE_FULLSCREEN, di);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--project")) {
        PROJECT project;
        strcpy(project.master_url, next_arg(argc, argv, i));
        canonicalize_master_url(project.master_url);
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "reset")) {
            retval = rpc.project_op(project, "reset");
        } else if (!strcmp(op, "suspend")) {
            retval = rpc.project_op(project, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.project_op(project, "resume");
        } else if (!strcmp(op, "detach")) {
            retval = rpc.project_op(project, "detach");
        } else if (!strcmp(op, "update")) {
            retval = rpc.project_op(project, "update");
        } else if (!strcmp(op, "suspend")) {
            retval = rpc.project_op(project, "suspend");
        } else if (!strcmp(op, "resume")) {
            retval = rpc.project_op(project, "resume");
        } else if (!strcmp(op, "nomorework")) {
            retval = rpc.project_op(project, "nomorework");
        } else if (!strcmp(op, "allowmorework")) {
            retval = rpc.project_op(project, "allowmorework");
        } else if (!strcmp(op, "detach_when_done")) {
            retval = rpc.project_op(project, "detach_when_done");
        } else if (!strcmp(op, "dont_detach_when_done")) {
            retval = rpc.project_op(project, "dont_detach_when_done");
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--project_attach")) {
        char url[256];
        strcpy(url, next_arg(argc, argv, i));
        canonicalize_master_url(url);
        char* auth = next_arg(argc, argv, i);
        retval = rpc.project_attach(url, auth, "");
    } else if (!strcmp(cmd, "--file_transfer")) {
        FILE_TRANSFER ft;

        ft.project_url = next_arg(argc, argv, i);
        ft.name = next_arg(argc, argv, i);
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "retry")) {
            retval = rpc.file_transfer_op(ft, "retry");
        } else if (!strcmp(op, "abort")) {
            retval = rpc.file_transfer_op(ft, "abort");
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--set_run_mode")) {
        char* op = next_arg(argc, argv, i);
        double duration;
        if (i >= argc || (argv[i][0] == '-')) {
            duration = 0;
        } else {
            duration = atof(next_arg(argc, argv, i));
        }
        if (!strcmp(op, "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS, duration);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO, duration);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER, duration);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--set_gpu_mode")) {
        char* op = next_arg(argc, argv, i);
        double duration;
        if (i >= argc || (argv[i][0] == '-')) {
            duration = 0;
        } else {
            duration = atof(next_arg(argc, argv, i));
        }
        if (!strcmp(op, "always")) {
            retval = rpc.set_gpu_mode(RUN_MODE_ALWAYS, duration);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_gpu_mode(RUN_MODE_AUTO, duration);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_gpu_mode(RUN_MODE_NEVER, duration);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--set_network_mode")) {
        char* op = next_arg(argc, argv, i);
        double duration;
        if (i >= argc || (argv[i][0] == '-')) {
            duration = 0;
        } else {
            duration = atof(next_arg(argc, argv, i));
        }
        if (!strcmp(op, "always")) {
            retval = rpc.set_network_mode(RUN_MODE_ALWAYS, duration);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_network_mode(RUN_MODE_AUTO, duration);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_network_mode(RUN_MODE_NEVER, duration);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--get_proxy_settings")) {
        GR_PROXY_INFO pi;
        retval = rpc.get_proxy_settings(pi);
        if (!retval) pi.print();
    } else if (!strcmp(cmd, "--set_proxy_settings")) {
        GR_PROXY_INFO pi;
        pi.http_server_name = next_arg(argc, argv, i);
        pi.http_server_port = atoi(next_arg(argc, argv, i));
        pi.http_user_name = next_arg(argc, argv, i);
        pi.http_user_passwd = next_arg(argc, argv, i);
        pi.socks_server_name = next_arg(argc, argv, i);
        pi.socks_server_port = atoi(next_arg(argc, argv, i));
        pi.socks_version = atoi(next_arg(argc, argv, i));
        pi.socks5_user_name = next_arg(argc, argv, i);
        pi.socks5_user_passwd = next_arg(argc, argv, i);
        pi.noproxy_hosts = next_arg(argc, argv, i);
        if (pi.http_server_name.size()) pi.use_http_proxy = true;
        if (pi.http_user_name.size()) pi.use_http_authentication = true;
        if (pi.socks_server_name.size()) pi.use_socks_proxy = true;
        retval = rpc.set_proxy_settings(pi);
    } else if (!strcmp(cmd, "--get_message_count")) {
        int seqno;
        retval = rpc.get_message_count(seqno);
        if (!retval) {
            printf("Greatest message sequence number: %d\n", seqno);
        }
    } else if (!strcmp(cmd, "--get_messages")) {
        int seqno;
        if (i == argc) {
            seqno = 0;
        } else {
            seqno = atoi(next_arg(argc, argv, i));
        }
        retval = rpc.get_messages(seqno, messages);
        if (!retval) {
            unsigned int j;
            for (j=0; j<messages.messages.size(); j++) {
                MESSAGE& md = *messages.messages[j];
                strip_whitespace(md.body);
                printf("%d: %s (%s) [%s] %s\n",
                    md.seqno,
                    time_to_string(md.timestamp),
                    prio_name(md.priority),
                    md.project.c_str(),
                    md.body.c_str()
                );
            }
        }
    } else if (!strcmp(cmd, "--get_notices")) {
        int seqno;
        if (i == argc) {
            seqno = 0;
        } else {
            seqno = atoi(next_arg(argc, argv, i));
        }
        retval = rpc.get_notices(seqno, notices);
        if (!retval) {
            unsigned int j;
            for (j=0; j<notices.notices.size(); j++) {
                NOTICE& n = *notices.notices[j];
                strip_whitespace(n.description);
                printf("%d: (%s) %s\n",
                    n.seqno,
                    time_to_string(n.create_time),
                    n.description.c_str()
                );
            }
        }
    } else if (!strcmp(cmd, "--get_host_info")) {
        HOST_INFO hi;
        retval = rpc.get_host_info(hi);
        if (!retval) hi.print();
    } else if (!strcmp(cmd, "--join_acct_mgr")) {
        char* am_url = next_arg(argc, argv, i);
        char* am_name = next_arg(argc, argv, i);
        char* am_passwd = next_arg(argc, argv, i);
        retval = rpc.acct_mgr_rpc(am_url, am_name, am_passwd);
        if (!retval) {
            while (1) {
                ACCT_MGR_RPC_REPLY amrr;
                retval = rpc.acct_mgr_rpc_poll(amrr);
                if (retval) {
                    printf("poll status: %s\n", boincerror(retval));
                } else {
                    if (amrr.error_num) {
                        printf("poll status: %s\n", boincerror(amrr.error_num));
                        if (amrr.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        int j, n = (int)amrr.messages.size();
                        if (n) {
                            printf("Messages from account manager:\n");
                            for (j=0; j<n; j++) {
                                printf("%s\n", amrr.messages[j].c_str());
                            }
                        }
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--quit_acct_mgr")) {
        retval = rpc.acct_mgr_rpc("", "", "");
    } else if (!strcmp(cmd, "--run_benchmarks")) {
        retval = rpc.run_benchmarks();
    } else if (!strcmp(cmd, "--get_project_config")) {
        char* gpc_url = next_arg(argc, argv,i);
        retval = rpc.get_project_config(string(gpc_url));
    } else if (!strcmp(cmd, "--get_project_config_poll")) {
        PROJECT_CONFIG pc;
        retval = rpc.get_project_config_poll(pc);
        if (retval) {
            printf("retval: %d\n", retval);
        } else {
            pc.print();
        }
    } else if (!strcmp(cmd, "--lookup_account")) {
        ACCOUNT_IN lai;
        lai.url = next_arg(argc, argv, i);
        lai.email_addr = next_arg(argc, argv, i);
        lai.passwd = next_arg(argc, argv, i);
        retval = rpc.lookup_account(lai);
        printf("status: %s\n", boincerror(retval));
        if (!retval) {
            ACCOUNT_OUT lao;
            while (1) {
                retval = rpc.lookup_account_poll(lao);
                if (retval) {
                    printf("poll status: %s\n", boincerror(retval));
                } else {
                    if (lao.error_num) {
                        printf("poll status: %s\n", boincerror(lao.error_num));
                        if (lao.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        lao.print();
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--create_account")) {
        ACCOUNT_IN cai;
        cai.url = next_arg(argc, argv, i);
        cai.email_addr = next_arg(argc, argv, i);
        cai.passwd = next_arg(argc, argv, i);
        cai.user_name = next_arg(argc, argv, i);
        retval = rpc.create_account(cai);
        printf("status: %s\n", boincerror(retval));
        if (!retval) {
            ACCOUNT_OUT lao;
            while (1) {
                retval = rpc.create_account_poll(lao);
                if (retval) {
                    printf("poll status: %s\n", boincerror(retval));
                } else {
                    if (lao.error_num) {
                        printf("poll status: %s\n", boincerror(lao.error_num));
                        if (lao.error_num != ERR_IN_PROGRESS) break;
                        boinc_sleep(1);
                    } else {
                        lao.print();
                        break;
                    }
                }
            }
        }
    } else if (!strcmp(cmd, "--read_global_prefs_override")) {
        retval = rpc.read_global_prefs_override();
    } else if (!strcmp(cmd, "--read_cc_config")) {
        retval = rpc.read_cc_config();
        printf("retval %d\n", retval);
    } else if (!strcmp(cmd, "--network_available")) {
        retval = rpc.network_available();
    } else if (!strcmp(cmd, "--get_cc_status")) {
        CC_STATUS cs;
        retval = rpc.get_cc_status(cs);
        if (!retval) {
            retval = cs.network_status;
        }
    } else if (!strcmp(cmd, "--set_debts")) {
        vector<PROJECT>projects;
        while (i < argc) {
            PROJECT proj;
            strcpy(proj.master_url, next_arg(argc, argv, i));
            int std = atoi(next_arg(argc, argv, i));
            proj.cpu_short_term_debt = std;
            proj.cuda_short_term_debt = std;
            proj.ati_short_term_debt = std;
            int ltd = atoi(next_arg(argc, argv, i));
            proj.cpu_long_term_debt = ltd;
            proj.cuda_debt = ltd;
            proj.ati_debt = ltd;
            projects.push_back(proj);
        }
        retval = rpc.set_debts(projects);
    } else if (!strcmp(cmd, "--quit")) {
        retval = rpc.quit();
    } else {
        fprintf(stderr, "unrecognized command %s\n", cmd);
    }
    if (retval < 0) {
        show_error(retval);
    }

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    exit(retval);
}

