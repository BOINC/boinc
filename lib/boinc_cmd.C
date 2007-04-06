// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// boinc_cmd: command-line interface to a BOINC core client,
// using GUI RPCs.
//
// usage: boinc_cmd [--host hostname] [--passwd passwd] command
//
// See help() below for a list of commands.

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include "config.h"
#include <cstdio>
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
#include "version.h"
#include "common_defs.h"


void usage() {
    fprintf(stderr, "\
Usage:  boinc_cmd [--host hostname] [--passwd passwd] command\n\
Give --help as a command for a list of commands\n\
");
    exit(1);
}

void version(){
    printf("boinc_cmd,  built from %s \n", PACKAGE_STRING );
    exit(0);
}

void help() {
    fprintf(stderr, "\n\n\
    usage: boinc_cmd [--host hostname] [--passwd passwd] command\n\n\
Commands:\n\
 --get_state                   show entire state\n\
 --get_results                 show results\n\
 --get_file_transfers            show file transfers\n\
 --get_project_status            show status of all attached projects\n\
 --get_simple_gui_info           show status of projects and active results\n\
 --get_disk_usage\n\
 --result url result_name {suspend | resume | abort | graphics_window | graphics_fullscreen}\n\
 --project url {reset | detach | update | suspend | resume | nomorework | allowmorework}\n\
 --project_attach url auth\n\
 --file_transfer url filename {retry | abort}\n\
 --set_run_mode {always | auto | never} duration\n\
 --set_network_mode {always | auto | never} duration\n\
 --get_proxy_settings\n\
 --set_proxy_settings\n\
 --get_messages seqno            show messages > seqno\n\
 --get_host_info\n\
 --acct_mgr_rpc url name password\n\
 --run_benchmarks\n\
 --get_screensaver_mode\n\
 --set_screensaver_mode on|off blank_time {desktop window_station}\n\
 --get_project_config url\n\
 --get_project_config_poll\n\
 --lookup_account url email passwd\n\
 --create_account url email passwd name\n\
 --quit\n\
   ");
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
        help();
        exit(1);
    }
    return argv[i++];
}


// If there's a password file, read it
//
void read_password_from_file(char* buf) {
	FILE* f = fopen("gui_rpc_auth.cfg", "r");
	if (!f) return;
	char* p = fgets(buf, 256, f);
        if (p) {                // Fixes compiler warning
            int n = (int)strlen(buf);

            // trim CR
            //
            if (n && buf[n-1]=='\n') {
                    buf[n-1] = 0;
            }
	}
	fclose(f);
}

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    int i, retval, port=0;
    MESSAGES messages;
	char passwd_buf[256], hostname_buf[256], *hostname=0;
    char* passwd = passwd_buf, *p;

    g_use_sandbox = false;

	strcpy(passwd_buf, "");
	read_password_from_file(passwd_buf);

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
    if (!strcmp(argv[i], "--help")) help();
    if (!strcmp(argv[i], "-h"))     help();
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
        fprintf(stderr, "can't connect to %s\n", hostname?hostname:"local host");
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
    } else if (!strcmp(cmd, "--get_results")) {
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
        SIMPLE_GUI_INFO sgi;
        retval = rpc.get_simple_gui_info(sgi);
        if (!retval) sgi.print();
    } else if (!strcmp(cmd, "--get_disk_usage")) {
        DISK_USAGE du;
        retval = rpc.get_disk_usage(du);
        if (!retval) du.print();
    } else if (!strcmp(cmd, "--result")) {
        RESULT result;
        char* project_url = next_arg(argc, argv, i);
        result.project_url = project_url;
        char* name = next_arg(argc, argv, i);
        result.name = name;
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
        project.master_url =  next_arg(argc, argv, i);
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
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--project_attach")) {
        char* url = next_arg(argc, argv, i);
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
        retval = rpc.set_proxy_settings(pi);
    } else if (!strcmp(cmd, "--get_messages")) {
        int seqno = atoi(next_arg(argc, argv, i));
        retval = rpc.get_messages(seqno, messages);
        if (!retval) {
            unsigned int j;
            for (j=0; j<messages.messages.size(); j++) {
                MESSAGE& md = *messages.messages[j];
                printf("%s %d %d %s\n",
                    md.project.c_str(), md.priority,
                    md.timestamp, md.body.c_str()
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
                        unsigned int i, n = amrr.messages.size();
                        if (n) {
                            printf("Messages from account manager:\n");
                            for (i=0; i<n; i++) {
                                printf("%s\n", amrr.messages[i].c_str());
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
    } else if (!strcmp(cmd, "--get_screensaver_mode")) {
        int status;
        retval = rpc.get_screensaver_mode(status);
        if (!retval) printf("screensaver mode: %d\n", status);
    } else if (!strcmp(cmd, "--set_screensaver_mode")) {
        double blank_time;
        bool enabled = false;
        DISPLAY_INFO di;

        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "on")) enabled = true;
        blank_time = atof(next_arg(argc, argv, i));
        parse_display_args(argv, i, di);
        retval = rpc.set_screensaver_mode(enabled, blank_time, di);
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
    } else if (!strcmp(cmd, "--test1")) {
        string s;
        retval = rpc.get_global_prefs_override(s);
        printf("retval: %d\nprefs:\n%s\n", retval, s.c_str());
    } else if (!strcmp(cmd, "--test2")) {
        string s = "foobar";
        retval = rpc.set_global_prefs_override(s);
        printf("retval: %d\n", retval);
    } else if (!strcmp(cmd, "--test3")) {
        GLOBAL_PREFS gp;
        GLOBAL_PREFS_MASK mask;
        memset(&gp, 0, sizeof(gp));
        mask.clear();
        retval = rpc.get_global_prefs_override_struct(gp, mask);
        printf("retval %d max %d\n", retval, gp.max_cpus);
    } else if (!strcmp(cmd, "--test4")) {
        GLOBAL_PREFS gp;
        GLOBAL_PREFS_MASK m;
        gp.max_cpus = 2;
        m.max_cpus = true;
        retval = rpc.set_global_prefs_override_struct(gp, m);
        printf("retval %d\n", retval);
    } else if (!strcmp(cmd, "--quit")) {
        retval = rpc.quit();
    } else {
        fprintf(stderr, "unrecognized command %s\n", cmd);
    }
    if (retval) {
        show_error(retval);
    }

#if defined(_WIN32) && defined(USE_WINSOCK)
    WSACleanup();
#endif
    return 0;
}

const char *BOINC_RCSID_77f00010ab = "$Id$";
