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

#ifdef _WIN32
#include "boinc_win.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <string.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "parse.h"

#include "file_names.h"
#include "client_msgs.h"
#include "client_state.h"

using std::string;
using std::list;
using std::vector;

#if defined(_WIN32)
typedef int socklen_t;
#elif defined ( __APPLE__)
typedef int32_t socklen_t;
#elif !GETSOCKOPT_SOCKLEN_T
#ifndef socklen_t
typedef size_t socklen_t;
#endif
#endif

GUI_RPC_CONN::GUI_RPC_CONN(int s) {
    sock = s;
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
#ifdef _WIN32
	closesocket(sock);
#else
    close(sock);
#endif
}

static void handle_get_project_status(MIOFILE& fout) {
    unsigned int i;
    fout.printf("<projects>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        p->write_state(fout);
    }
    fout.printf("</projects>\n");
}

static void handle_get_disk_usage(MIOFILE& fout) {
    unsigned int i;
    double size;

    fout.printf("<projects>\n");
    for (i=0; i<gstate.projects.size(); i++) {
        PROJECT* p = gstate.projects[i];
        gstate.project_disk_usage(p, size);
        fout.printf(
            "<project>\n"
            "  <master_url>%s</master_url>\n"
            "  <disk_usage>%f</disk_usage>\n"
            "</project>\n",
            p->master_url, size
        );
    }
    fout.printf("</projects>\n");
}

static PROJECT* get_project(char* buf, MIOFILE& fout) {
	string url;
	if (!parse_str(buf, "<project_url>", url)) {
		fout.printf("<error>Missing project URL</error>\n");
		return 0;
	}
	PROJECT* p = gstate.lookup_project(url.c_str());
	if (!p) {
		fout.printf("<error>No such project</error>\n");
		return 0 ;
	}
	return p;
}

static void handle_result_show_graphics(char* buf, MIOFILE& fout) {
    string result_name;
    ACTIVE_TASK* atp;
    int mode;

    if (match_tag(buf, "<full_screen/>")) {
        mode = MODE_FULLSCREEN;
    } else {
        mode = MODE_WINDOW;
    }

	if (parse_str(buf, "<result_name>", result_name)) {
        PROJECT* p = get_project(buf, fout);
        if (!p) {
            fout.printf("<error>No such project</error>\n");
            return;
       }
        RESULT* rp = gstate.lookup_result(p, result_name.c_str());
        if (!rp) {
            fout.printf("<error>No such result</error>\n");
            return;
        }
        atp = gstate.lookup_active_task_by_result(rp);
        if (!atp || atp->scheduler_state != CPU_SCHED_RUNNING) {
            fout.printf("<error>Result not active</error>\n");
            return;
        }
        atp->request_graphics_mode(mode);
    } else {
        for (unsigned int i=0; i<gstate.active_tasks.active_tasks.size(); i++) {
            atp = gstate.active_tasks.active_tasks[i];
            if (atp->scheduler_state != CPU_SCHED_RUNNING) continue;
            atp->request_graphics_mode(mode);
        }
    }
	fout.printf("<success/>\n");
}


static void handle_project_reset(char* buf, MIOFILE& fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
		gstate.reset_project(p);
		fout.printf("<success/>\n");
	}
}

static void handle_project_attach(char* buf, MIOFILE& fout) {
	string url, authenticator;
	if (!parse_str(buf, "<project_url>", url)) {
		fout.printf("<error>Missing URL</error>\n");
		return;
	}
	if (!parse_str(buf, "<authenticator>", authenticator)) {
		fout.printf("<error>Missing authenticator</error>\n");
		return;
	}
	gstate.add_project(url.c_str(), authenticator.c_str());
	fout.printf("<success/>\n");
}

static void handle_project_detach(char* buf, MIOFILE& fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
		gstate.detach_project(p);
		fout.printf("<success/>\n");
	}
}

static void handle_project_update(char* buf, MIOFILE& fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
        p->sched_rpc_pending = true;
        p->min_rpc_time = 0;
		fout.printf("<success/>\n");
	}
}

static void handle_set_run_mode(char* buf, MIOFILE& fout) {
	if (match_tag(buf, "<always")) {
		gstate.user_run_request = USER_RUN_REQUEST_ALWAYS;
	} else if (match_tag(buf, "<never")) {
		gstate.user_run_request = USER_RUN_REQUEST_NEVER;
	} else if (match_tag(buf, "<auto")) {
		gstate.user_run_request = USER_RUN_REQUEST_AUTO;
	} else {
		fout.printf("<error>Missing mode</error>\n");
		return;
	}
	fout.printf("<success/>\n");
}

static void handle_run_benchmarks(char* buf, MIOFILE& fout) {
    // TODO: suspend activities; make sure run at right priority
    //
    gstate.start_cpu_benchmarks();
    fout.printf("<success/>\n");
}

static void handle_set_proxy_settings(char* buf, MIOFILE& fout) {
    string socks_proxy_server_name,http_proxy_server_name;
    int socks_proxy_server_port,http_proxy_server_port;
    if (!parse_str(buf, "<socks_proxy_server_name>", socks_proxy_server_name)) {
        fout.printf("<error>SOCKS proxy server name missing</error>\n");
        return;
    }
    if (!parse_int(buf, "<socks_proxy_server_port>", socks_proxy_server_port)) {
        fout.printf("<error>SOCKS proxy server port missing</error>\n");
        return;
    }
    if (!parse_str(buf, "<http_proxy_server_name>", http_proxy_server_name)) {
        fout.printf("<error>HTTP proxy server name missing</error>\n");
        return;
    }
    if (!parse_int(buf, "<http_proxy_server_port>", http_proxy_server_port)) {
        fout.printf("<error>HTTP proxy server port missing</error>\n");
        return;
    }
    safe_strcpy(gstate.pi.socks_server_name, socks_proxy_server_name.c_str());
    gstate.pi.socks_server_port = socks_proxy_server_port;
    safe_strcpy(gstate.pi.http_server_name, http_proxy_server_name.c_str());
    gstate.pi.http_server_port = http_proxy_server_port;
    fout.printf("<success/>\n");
}

// params:
// <nmessages>x</nmessages>
//    return at most this many messages
// <offset>n</offset>
//    start at message n.
// if no offset is given, return last n messages
//
void handle_get_messages(char* buf, MIOFILE& fout) {
    int nmessages=-1, seqno=-1, j;

    parse_int(buf, "<nmessages>", nmessages);
    parse_int(buf, "<seqno>", seqno);
    if (nmessages < 0) {
        fout.printf("<error>No nmessages given</error>\n");
        return;
    }

    fout.printf("<msgs>\n");
    list<MESSAGE_DESC*>::const_iterator iter;
    for (iter=message_descs.begin(), j=0;
        iter != message_descs.end() && j<nmessages;
        iter++, j++
    ) {
        MESSAGE_DESC* mdp = *iter;
        if (mdp->seqno <= seqno) break;
        fout.printf(
            "<msg>\n"
            " <pri>%d</pri>\n"
            " <seqno>%d</seqno>\n"
            " <body>\n%s\n</body>\n"
            " <time>%d</time>\n",
            mdp->priority,
            mdp->seqno,
            mdp->message.c_str(),
            mdp->timestamp
        );
        if (mdp->project) {
            fout.printf(
                " <project>%s</project>\n",
                mdp->project->get_project_name()
            );
        }
        fout.printf("</msg>\n");
    }
    fout.printf("</msgs>\n");
}

int GUI_RPC_CONN::handle_rpc() {
    char request_msg[1024];
    int n;
    MIOFILE mf;
    MFILE m;
    char* p;
    mf.init_mfile(&m);

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_GUIRPC);

    // read the request message in one read()
	// so that the core client won't hang because
	// of malformed request msgs
	//
#ifdef _WIN32
        n = recv(sock, request_msg, 1024, 0);
#else
        n = read(sock, request_msg, 1024);
#endif
    if (n <= 0) return -1;
    request_msg[n] = 0;

    scope_messages.printf("GUI RPC Command = '%s'\n", request_msg);

    if (match_tag(request_msg, "<get_state")) {
        gstate.write_state_gui(mf);
	} else if (match_tag(request_msg, "<get_tasks>")) {
        gstate.write_tasks_gui(mf);
	} else if (match_tag(request_msg, "<get_file_transfers>")) {
        gstate.write_file_transfers_gui(mf);
	} else if (match_tag(request_msg, "<get_project_status>")) {
		handle_get_project_status(mf);
	} else if (match_tag(request_msg, "<get_disk_usage>")) {
		handle_get_disk_usage(mf);
	} else if (match_tag(request_msg, "<result_show_graphics>")) {
		handle_result_show_graphics(request_msg, mf);
	} else if (match_tag(request_msg, "<project_reset>")) {
		handle_project_reset(request_msg, mf);
	} else if (match_tag(request_msg, "<project_attach>")) {
		handle_project_attach(request_msg, mf);
	} else if (match_tag(request_msg, "<project_detach>")) {
		handle_project_detach(request_msg, mf);
	} else if (match_tag(request_msg, "<project_update>")) {
		handle_project_update(request_msg, mf);
	} else if (match_tag(request_msg, "<set_run_mode>")) {
		handle_set_run_mode(request_msg, mf);
	} else if (match_tag(request_msg, "<run_benchmarks")) {
		handle_run_benchmarks(request_msg, mf);
	} else if (match_tag(request_msg, "<set_proxy_settings>")) {
		handle_set_proxy_settings(request_msg, mf);
	} else if (match_tag(request_msg, "<get_messages>")) {
		handle_get_messages(request_msg, mf);
    } else {
        mf.printf("<unrecognized/>\n");
    }

    mf.printf("\003");
    m.get_buf(p, n);
    send(sock, p, n, 0);
    if (p) free(p);

    return 0;
}

int GUI_RPC_CONN_SET::get_allowed_hosts() {
 
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_STATE);
     
    // add localhost
    allowed_remote_ip_addresses.push_back(0x7f000001);
     
    NET_XFER temp; // network address resolver is in this class
    int ipaddr;
    char buf[256];
 
    // open file remote_hosts.cfg and read in the
    // allowed host list and resolve them to an ip address
    //
    FILE* f = fopen(REMOTEHOST_FILE_NAME, "r");
    if (f != NULL) {    
         scope_messages.printf(
            "GUI_RPC_CONN_SET::get_allowed_hosts(): found allowed hosts list\n"
        );
 
        // read in each line, if it is not a comment
        // then resolve the address and add to our
        // allowed list
        memset(buf,0,sizeof(buf));
        while (fgets(buf, 256, f) != NULL) {
            strip_whitespace(buf);
            if (!(buf[0] =='#' || buf[0] == ';') && strlen(buf) > 0 ) {
                // resolve and add
                if (temp.get_ip_addr(buf, ipaddr) == 0) {
                    allowed_remote_ip_addresses.push_back((int)ntohl(ipaddr));
               }
            }
        }
        fclose(f);
    }
    return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

int GUI_RPC_CONN_SET::init() {
	sockaddr_in addr;
    int retval;

    // get list of hosts allowed to do GUI RPCs
    //
    get_allowed_hosts();

    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) {
        msg_printf(NULL, MSG_ERROR,
            "GUI RPC failed to create socket: %d\n", lsock
        );
        return ERR_SOCKET;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int one = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (char*)&one, 4);

    retval = bind(lsock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "GUI RPC bind failed: %d\n", retval);
        return ERR_BIND;
    }
    retval = listen(lsock, 999);
    if (retval) {
        msg_printf(NULL, MSG_ERROR, "GUI RPC listen failed: %d\n", retval);
        return ERR_LISTEN;
    }
    return 0;
}

bool GUI_RPC_CONN_SET::poll() {
    int n = 0;

    if (lsock >= 0) {
        unsigned int i;
        fd_set read_fds, error_fds;
        int sock, retval;
        vector<GUI_RPC_CONN*>::iterator iter;
        GUI_RPC_CONN* gr;
        struct timeval tv;

        FD_ZERO(&read_fds);
        FD_ZERO(&error_fds);
        FD_SET(lsock, &read_fds);
        for (i=0; i<gui_rpcs.size(); i++) {
            gr = gui_rpcs[i];
            FD_SET(gr->sock, &read_fds);
            FD_SET(gr->sock, &error_fds);
        }

        memset(&tv, 0, sizeof(tv));
        n = select(FD_SETSIZE, &read_fds, 0, &error_fds, &tv);
        if (FD_ISSET(lsock, &read_fds)) {
            struct sockaddr_in addr;

            socklen_t addr_len = sizeof(addr);
            sock = accept(lsock, (struct sockaddr*)&addr, &addr_len);

            int peer_ip = (int) ntohl(addr.sin_addr.s_addr);

            // check list of allowed remote hosts
            bool allowed = false;
            vector<int>::iterator remote_iter;
 
            remote_iter = allowed_remote_ip_addresses.begin();
            while (remote_iter != allowed_remote_ip_addresses.end() ) {
                int remote_host = *remote_iter;
                if (peer_ip == remote_host) allowed = true;
                remote_iter++;
            }
             
            // accept the connection if:
            // 1) allow_remote_gui_rpc is set or
            // 2) client host is included in "remote_hosts" file or
            // 3) client is on localhost
            //
            if ( !(gstate.allow_remote_gui_rpc) && !(allowed)) {
                in_addr ia;
                ia.s_addr = peer_ip;
                msg_printf(
                    NULL, MSG_ERROR,
                    "GUI RPC request from non-allowed address %s\n",
                    inet_ntoa(htonl(ia))
                );
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
            } else {
                GUI_RPC_CONN* gr = new GUI_RPC_CONN(sock);
                insert(gr);
            }
        }
        iter = gui_rpcs.begin();
        while (iter != gui_rpcs.end()) {
            gr = *iter;
            if (FD_ISSET(gr->sock, &error_fds)) {
                delete gr;
                gui_rpcs.erase(iter);
            } else {
                iter++;
            }
        }
        iter = gui_rpcs.begin();
        while (iter != gui_rpcs.end()) {
            gr = *iter;
            if (FD_ISSET(gr->sock, &read_fds)) {
                retval = gr->handle_rpc();
                if (retval) {
                    delete gr;
                    gui_rpcs.erase(iter);
                    continue;
                }
            }
            iter++;
        }
    }
    return (n != 0);
}
