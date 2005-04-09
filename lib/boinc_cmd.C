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
// commands:
// --get_state               	show entire state
// --get_results             	show results
// --get_file_transfers			show file transfers
// --get_project_status			show status of all projects
// --get_disk_usage
// --result
//      {suspend | resume | abort | graphics_window | graphics_fullscreen}
//      url result_name
// --project
//		{reset | detach | update | suspend | resume | nomorework | allowmorework}
//      url
// --project_attach url auth
// --file_transfer {retry | abort} url filename
// --get_run_mode
// --set_run_mode {always | auto | never}
// --get_network_mode
// --set_network_mode {always | auto | never}
// --get_proxy_settings
// --set_proxy_settings
// --get_messages seqno			show messages > seqno
// --get_host_info
// --acct_mgr_rpc url name password
// --run_benchmarks
// --get_screensaver_mode
// --set_screensaver_mode on|off blank_time {desktop window_station}
// --quit


#ifdef _WIN32
#include "boinc_win.h"
#include "win_net.h"
#else
#include <cstdio>
#include <unistd.h>
#endif

#include <vector>
using std::vector;

#include "gui_rpc_client.h"
#include "error_numbers.h"

void usage() {
    fprintf(stderr, "bad usage\n");
    exit(1);
}

void parse_display_args(char** argv, int& i, DISPLAY_INFO& di) {
    strcpy(di.window_station, "winsta0");
    strcpy(di.desktop, "default");
    strcpy(di.display, "");
    while (argv[i]) {
        if (!strcmp(argv[i], "--window_station")) {
            strcpy(di.window_station, argv[++i]);
        } else if (!strcpy(argv[i], "--desktop")) {
            strcpy(di.desktop, argv[++i]);
        } else if (!strcpy(argv[i], "--display")) {
            strcpy(di.display, argv[++i]);
        }
        i++;
    }
}

void show_error(int retval) {
    switch(retval) {
    case ERR_AUTHENTICATOR:
        fprintf(stderr, "Authentication failure\n");
        break;
    default:
        fprintf(stderr, "Error %d\n", retval);
    }
}

char* next_arg(int argc, char** argv, int& i) {
    if (i >= argc) {
        fprintf(stderr, "Missing command-line argument\n");
        exit(1);
    }
    return argv[i++];
}

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    int i;
    MESSAGES messages;
    int retval;
    char* hostname = NULL;
    char* passwd = NULL;

#ifdef _WIN32
    WSADATA wsdata;
    retval = WSAStartup( MAKEWORD( 1, 1 ), &wsdata);
    if (retval) {
        fprintf(stderr, "WinsockInitialize: %d\n", retval);
        exit(1);
    }
#endif
    if (argc < 2) usage();
    i = 1;
    if (!strcmp(argv[i], "--host")) {
        hostname = argv[i+1];
        i += 2;
    }
    if (!strcmp(argv[i], "--passwd")) {
        passwd = argv[i+1];
        i += 2;
    }
#if 1
    retval = rpc.init(hostname, false);
    if (retval) {
        fprintf(stderr, "can't connect\n");
        exit(1);
    }
#else
    retval = rpc.init(hostname, true);
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
    } else if (!strcmp(cmd, "--get_disk_usage")) {
        PROJECTS ps;
        retval = rpc.get_disk_usage(ps);
        if (!retval) ps.print();
    } else if (!strcmp(cmd, "--result")) {
        RESULT result;
        char* project_url = next_arg(argc, argv, i);
        char* name = next_arg(argc, argv, i);
        result.project_url = project_url;
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
            retval = rpc.show_graphics(project_url, name, false, di);
        } else if (!strcmp(op, "graphics_fullscreen")) {
            DISPLAY_INFO di;
            parse_display_args(argv, i, di);
            retval = rpc.show_graphics(project_url, name, true, di);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--project")) {
        PROJECT project;
        project.master_url =  next_arg(argc, argv, i);
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "reset")) {
            retval = rpc.project_op(project, "reset");
        } else if (!strcmp(op, "detach")) {
            retval = rpc.project_op(project, "detach");
        } else if (!strcmp(op, "update")) {
            retval = rpc.project_op(project, "update");
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
        retval = rpc.project_attach(url, auth);
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
    } else if (!strcmp(cmd, "--get_run_mode")) {
        int mode;
        retval = rpc.get_run_mode(mode);
        if (!retval) {
            printf("run mode: %s\n", rpc.mode_name(mode));
        }
    } else if (!strcmp(cmd, "--set_run_mode")) {
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--get_network_mode")) {
        int mode;
        retval = rpc.get_network_mode(mode);
        if (!retval) {
            printf("network mode: %s\n", rpc.mode_name(mode));
        }
    } else if (!strcmp(cmd, "--set_network_mode")) {
        char* op = next_arg(argc, argv, i);
        if (!strcmp(op, "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
        } else if (!strcmp(op, "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO);
        } else if (!strcmp(op, "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER);
        } else {
            fprintf(stderr, "Unknown op %s\n", op);
        }
    } else if (!strcmp(cmd, "--get_proxy_settings")) {
        PROXY_INFO pi;
        retval = rpc.get_proxy_settings(pi);
        if (!retval) pi.print();
    } else if (!strcmp(cmd, "--set_proxy_settings")) {
        PROXY_INFO pi;
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
    } else if (!strcmp(cmd, "--acct_mgr_rpc")) {
        char* am_url = next_arg(argc, argv, i);
        char* am_name = next_arg(argc, argv, i);
        char* am_passwd = next_arg(argc, argv, i);
        retval = rpc.acct_mgr_rpc(am_url, am_name, am_passwd);
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
    } else if (!strcmp(cmd, "--quit")) {
        retval = rpc.quit();
    } else {
        fprintf(stderr, "unrecognized command %s\n", cmd);
    }
    if (retval) {
        show_error(retval);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

const char *BOINC_RCSID_77f00010ab = "$Id$";
