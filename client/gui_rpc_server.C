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
#include "stdafx.h"
#endif

#ifndef _WIN32
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>
#include <string.h>
#endif

#include "parse.h"
#include "client_state.h"

GUI_RPC_CONN::GUI_RPC_CONN(int s) {
    sock = s;
#ifdef _WIN32
	fout = _fdopen(_dup(sock), "w");
#else
    fout = fdopen(dup(sock), "w");
#endif
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
#ifdef _WIN32
	closesocket(sock);
#else
    close(sock);
    fclose(fout);
#endif
}

static PROJECT* get_project(char* buf, FILE* fout) {
	string url;
	if (!parse_str(buf, "<project_url>", url)) {
		fprintf(fout, "<error>Missing project URL</error>\n");
		return 0;
	}
	PROJECT* p = gstate.lookup_project(url.c_str());
	if (!p) {
		fprintf(fout, "<error>No such project</error>\n");
		return 0 ;
	}
	return p;
}

static void handle_result_show_graphics(char* buf, FILE* fout) {
	string result_name;
    PROJECT* p = get_project(buf, fout);
    if (!p) return;

	if (!parse_str(buf, "<result_name>", result_name)) {
		fprintf(fout, "<error>Missing result name</error>\n");
		return;
	}
	RESULT* rp = gstate.lookup_result(p, result_name.c_str());
	if (!rp) {
		fprintf(fout, "<error>No such result</error>\n");
		return;
	}
	ACTIVE_TASK* atp = gstate.lookup_active_task_by_result(rp);
    if (!atp) {
		fprintf(fout, "<error>Result not active</error>\n");
		return;
	}
    atp->request_graphics_mode(MODE_WINDOW);
	fprintf(fout, "<success/>\n");
}


static void handle_project_reset(char* buf, FILE* fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
		gstate.reset_project(p);
		fprintf(fout, "<success/>\n");
	}
}

static void handle_project_attach(char* buf, FILE* fout) {
	string url, authenticator;
	if (!parse_str(buf, "<url>", url)) {
		fprintf(fout, "<error>Missing URL</error>\n");
		return;
	}
	if (!parse_str(buf, "<authenticator>", authenticator)) {
		fprintf(fout, "<error>Missing authenticator</error>\n");
		return;
	}
	gstate.add_project(url.c_str(), authenticator.c_str());
	fprintf(fout, "<success/>\n");
}

static void handle_project_detach(char* buf, FILE* fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
		gstate.detach_project(p);
		fprintf(fout, "<success/>\n");
	}
}

static void handle_project_update(char* buf, FILE* fout) {
	PROJECT* p = get_project(buf, fout);
	if (p) {
        p->sched_rpc_pending = true;
        p->min_rpc_time = 0;
		fprintf(fout, "<success/>\n");
	}
}

static void handle_set_run_mode(char* buf, FILE* fout) {
	if (match_tag(buf, "<always>")) {
		gstate.user_run_request = USER_RUN_REQUEST_ALWAYS;
	} else if (match_tag(buf, "<never>")) {
		gstate.user_run_request = USER_RUN_REQUEST_NEVER;
	} else if (match_tag(buf, "<auto>")) {
		gstate.user_run_request = USER_RUN_REQUEST_AUTO;
	} else {
		fprintf(fout, "<error>Missing mode</error>\n");
		return;
	}
	fprintf(fout, "<success/>\n");
}

static void handle_run_benchmarks(char* buf, FILE* fout) {
    // TODO: suspend activities; make sure run at right priority
    //
    gstate.start_cpu_benchmarks();
    fprintf(fout, "<success/>\n");
}

static void handle_set_proxy_settings(char* buf, FILE* fout) {
    string socks_proxy_server_name,http_proxy_server_name;
    int socks_proxy_server_port,http_proxy_server_port;
    if (!parse_str(buf, "<socks_proxy_server_name>", socks_proxy_server_name)) {
        fprintf(fout, "<error>SOCKS proxy server name missing</error>\n");
        return;
    }
    if (!parse_int(buf, "<socks_proxy_server_port>", socks_proxy_server_port)) {
        fprintf(fout, "<error>SOCKS proxy server port missing</error>\n");
        return;
    }
    if (!parse_str(buf, "<http_proxy_server_name>", http_proxy_server_name)) {
        fprintf(fout, "<error>HTTP proxy server name missing</error>\n");
        return;
    }
    if (!parse_int(buf, "<http_proxy_server_port>", http_proxy_server_port)) {
        fprintf(fout, "<error>HTTP proxy server port missing</error>\n");
        return;
    }
    safe_strcpy(gstate.pi.socks_server_name, socks_proxy_server_name.c_str());
    gstate.pi.socks_server_port = socks_proxy_server_port;
    safe_strcpy(gstate.pi.http_server_name, http_proxy_server_name.c_str());
    gstate.pi.http_server_port = http_proxy_server_port;
    fprintf(fout, "<success/>\n");
}

int GUI_RPC_CONN::handle_rpc() {
    char buf[1024];
    int n;

	// read the request message in one read()
	// so that the core client won't hang because
	// of malformed request msgs
	//
    n = read(sock, buf, 1024);
    if (n <= 0) return -1;
    buf[n] = 0;
    printf("got %s\n", buf);
    if (match_tag(buf, "<get_state")) {
        gstate.write_state(fout);
	} else if (match_tag(buf, "<result_show_graphics>")) {
		handle_result_show_graphics(buf, fout);
	} else if (match_tag(buf, "<project_reset>")) {
		handle_project_reset(buf, fout);
	} else if (match_tag(buf, "<project_attach>")) {
		handle_project_attach(buf, fout);
	} else if (match_tag(buf, "<project_detach>")) {
		handle_project_detach(buf, fout);
	} else if (match_tag(buf, "<project_update>")) {
		handle_project_update(buf, fout);
	} else if (match_tag(buf, "<set_run_mode>")) {
		handle_set_run_mode(buf, fout);
	} else if (match_tag(buf, "<run_benchmarks>")) {
		handle_run_benchmarks(buf, fout);
	} else if (match_tag(buf, "<set_proxy_settings>")) {
		handle_set_proxy_settings(buf, fout);
    } else {
        fprintf(fout, "<unrecognized/>\n");
    }
    fflush(fout);
   return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

void GUI_RPC_CONN_SET::init(char* path) {
#ifdef _WIN32
	sockaddr addr;
#else
    sockaddr_un addr;
#endif
    int retval;

    unlink(path);
#ifdef _WIN32
	addr.sa_family = AF_UNIX;
	strcpy(addr.sa_data, path);
	WSADATA wsdata;
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSAStartup(wVersionRequested, &wsdata);
#else
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
#endif
    lsock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lsock < 0) {
        perror("socket");
        exit(1);
    }
    retval = bind(lsock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        perror("bind");
        exit(1);
    }
    retval = listen(lsock, 999);
    if (retval) {
        perror("listen");
        exit(1);
    }
}

bool GUI_RPC_CONN_SET::poll() {
    unsigned int i;
    fd_set read_fds, error_fds;
    int sock, n, retval;
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
        sock = accept(lsock, 0, 0);
        GUI_RPC_CONN* gr = new GUI_RPC_CONN(sock);
        insert(gr);
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
    return (n != 0);
}
