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
// usage: boinc_cmd [--host hostname] command
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
#endif
#include <vector>
using std::vector;

#ifndef _WIN32
#include <cstdio>
#endif

#include "gui_rpc_client.h"
#include "error_numbers.h"

void usage() {
    fprintf(stderr, "bad usage\n");
    exit(1);
}

int main(int argc, char** argv) {
    RPC_CLIENT rpc;
    unsigned int i;
    MESSAGES messages;
    int retval;
    char* hostname=0;
    PROJECT project;

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
    if (!strcmp(argv[1], "-host")) {
        hostname = argv[2];
        i = 3;
    }
    retval = rpc.init(hostname);
    if (retval) {
        fprintf(stderr, "can't connect\n");
        exit(1);
    }

    if (!strcmp(argv[i], "--get_state")) {
        CC_STATE state;
        retval = rpc.get_state(state);
        if (!retval) state.print();
    } else if (!strcmp(argv[i], "--get_results")) {
        RESULTS results;
        retval = rpc.get_results(results);
        if (!retval) results.print();
    } else if (!strcmp(argv[i], "--get_file_transfers")) {
        FILE_TRANSFERS ft;
        retval = rpc.get_file_transfers(ft);
        if (!retval) ft.print();
    } else if (!strcmp(argv[i], "--get_project_status")) {
        PROJECTS ps;
        retval = rpc.get_project_status(ps);
        if (!retval) ps.print();
    } else if (!strcmp(argv[i], "--get_disk_usage")) {
        PROJECTS ps;
        retval = rpc.get_disk_usage(ps);
        if (!retval) ps.print();
    } else if (!strcmp(argv[i], "--result")) {
        RESULT result;
        i++;
        result.project_url = argv[i+1];
        result.name = argv[i+2];
        if (!strcmp(argv[i], "suspend")) {
            retval = rpc.result_op(result, "suspend");
        }
        if (!strcmp(argv[i], "resume")) {
            retval = rpc.result_op(result, "resume");
        }
        if (!strcmp(argv[i], "abort")) {
            retval = rpc.result_op(result, "abort");
        }
        if (!strcmp(argv[i], "graphics_window")) {
            retval = rpc.show_graphics(argv[i+1], argv[i+2], false, "winsta0", "default");
        }
        if (!strcmp(argv[i], "graphics_fullscreen")) {
            retval = rpc.show_graphics(argv[i+1], argv[i+2], true, "winsta0", "default");
        }
    } else if (!strcmp(argv[i], "--project")) {
        PROJECT project;
        i++;
        project.master_url = argv[i+1];
        if (!strcmp(argv[i], "reset")) {
            retval = rpc.project_op(project, "reset");
        }
        if (!strcmp(argv[i], "detach")) {
            retval = rpc.project_op(project, "detach");
        }
        if (!strcmp(argv[i], "update")) {
            retval = rpc.project_op(project, "update");
        }
        if (!strcmp(argv[i], "nomorework")) {
            retval = rpc.project_op(project, "nomorework");
        }
        if (!strcmp(argv[i], "allowmorework")) {
            retval = rpc.project_op(project, "allowmorework");
        }
    } else if (!strcmp(argv[i], "--project_attach")) {
        retval = rpc.project_attach(argv[++i], argv[++i]);
    } else if (!strcmp(argv[i], "--file_transfer")) {
        FILE_TRANSFER ft;

        i++;
        ft.project_url = argv[i+1];
        ft.name = argv[i+2];
        if (!strcmp(argv[i], "retry")) {
            retval = rpc.file_transfer_op(ft, "retry");
        }
        if (!strcmp(argv[i], "abort")) {
            retval = rpc.file_transfer_op(ft, "abort");
        }
    } else if (!strcmp(argv[i], "--get_run_mode")) {
        int mode;
        retval = rpc.get_run_mode(mode);
        if (!retval) {
            printf("run mode: %s\n", rpc.mode_name(mode));
        }
    } else if (!strcmp(argv[i], "--set_run_mode")) {
        i++;
        if (!strcmp(argv[i], "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
        } else if (!strcmp(argv[i], "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO);
        } else if (!strcmp(argv[i], "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER);
        }
    } else if (!strcmp(argv[i], "--get_network_mode")) {
        int mode;
        retval = rpc.get_network_mode(mode);
        if (!retval) {
            printf("network mode: %s\n", rpc.mode_name(mode));
        }
    } else if (!strcmp(argv[i], "--set_network_mode")) {
        i++;
        if (!strcmp(argv[i], "always")) {
            retval = rpc.set_run_mode(RUN_MODE_ALWAYS);
        } else if (!strcmp(argv[i], "auto")) {
            retval = rpc.set_run_mode(RUN_MODE_AUTO);
        } else if (!strcmp(argv[i], "never")) {
            retval = rpc.set_run_mode(RUN_MODE_NEVER);
        }
    } else if (!strcmp(argv[i], "--get_proxy_settings")) {
        PROXY_INFO pi;
        retval = rpc.get_proxy_settings(pi);
        if (!retval) pi.print();
    } else if (!strcmp(argv[i], "--set_proxy_settings")) {
        PROXY_INFO pi;
        i++;
        pi.http_server_name = argv[i++];
        pi.http_server_port = atoi(argv[i++]);
        pi.http_user_name = argv[i++];
        pi.http_user_passwd = argv[i++];
        pi.socks_server_name = argv[i++];
        pi.socks_server_port = atoi(argv[i++]);
        pi.socks_version = atoi(argv[i++]);
        pi.socks5_user_name = argv[i++];
        pi.socks5_user_passwd = argv[i++];
        retval = rpc.set_proxy_settings(pi);
    } else if (!strcmp(argv[i], "--get_messages")) {
        int seqno = atoi(argv[++i]);
        retval = rpc.get_messages(seqno, messages);
        if (!retval) {
            for (i=0; i<messages.messages.size(); i++) {
                MESSAGE& md = *messages.messages[i];
                printf("%s %d %d %s\n",
                    md.project.c_str(), md.priority,
                    md.timestamp, md.body.c_str()
                );
            }
        }
    } else if (!strcmp(argv[i], "--get_host_info")) {
        HOST_INFO hi;
        retval = rpc.get_host_info(hi);
        if (!retval) hi.print();
    } else if (!strcmp(argv[i], "--acct_mgr_rpc")) {
        i++;
        retval = rpc.acct_mgr_rpc(argv[i], argv[i+1], argv[i+2]);
    } else if (!strcmp(argv[i], "--run_benchmarks")) {
        retval = rpc.run_benchmarks();
    } else if (!strcmp(argv[i], "--get_screensaver_mode")) {
        int status;
        retval = rpc.get_screensaver_mode(status);
        if (!retval) printf("screensaver mode: %d\n", status);
    } else if (!strcmp(argv[i], "--set_screensaver_mode")) {
        double blank_time;
        bool enabled = false;
        i++;
        if (!strcmp(argv[i], "on")) enabled = true;
        blank_time = atof(argv[i+3]);
        retval = rpc.set_screensaver_mode(enabled, argv[i+1], argv[i+2], blank_time);

    } else if (!strcmp(argv[i], "--quit")) {
        retval = rpc.quit();
    } else {
        fprintf(stderr, "unrecognized command %s\n", argv[i]);
    }
    if (retval) {
        fprintf(stderr, "Operation failed: %d\n", retval);
    }

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

const char *BOINC_RCSID_77f00010ab = "$Id$";
