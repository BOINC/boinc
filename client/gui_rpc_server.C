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
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "parse.h"

#include "client_msgs.h"
#include "client_state.h"

GUI_RPC_CONN::GUI_RPC_CONN(int s) {
    sock = s;
    
#ifdef _WIN32
    fout = fdopen(dup(_open_osfhandle(sock, _O_RDWR | _O_BINARY)), "w");
    setvbuf(fout, buffer, _IOFBF, 262144);
    memset(buffer, NULL, 262144);
#else
    fout = fdopen(dup(sock), "w");
#endif
}

GUI_RPC_CONN::~GUI_RPC_CONN() {
    fclose(fout);
#ifdef _WIN32
	closesocket(sock);
#else
    close(sock);
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

// params:
// <nmessages>x</nmessages>
//    return at most this many messages
// <offset>n</offset>
//    start at message n.
// if no offset is given, return last n messages
//
void handle_get_messages(char* buf, FILE* fout) {
    int nmessages=-1, offset=-1, j;
    unsigned int i;

    parse_int(buf, "<nmessages>", nmessages);
    parse_int(buf, "<offset>", offset);
    if (nmessages < 0) {
        fprintf(fout, "<error>No nmessages given</error>\n");
        return;
    }

    if (offset > (int)message_descs.size()) {
        offset = message_descs.size();
    }
    if (offset < 0) {
        offset = message_descs.size()-nmessages;
        if (offset < 0) {
            offset = 0;
        }
    }

    fprintf(fout, "<msgs>\n");
    j = 0;
    for (i=offset; i<message_descs.size()&&j<nmessages; i++, j++) {
        MESSAGE_DESC& md = message_descs[i];
        fprintf(fout,
            "<msg>\n"
            " <i>%d</i>\n"
            " <pri>%d</pri>\n"
            " <body>%s</body>\n"
            " <time>%d</time>\n",
            i,
            md.priority,
            md.message.c_str(),
            md.timestamp
        );
        if (md.project) {
            fprintf(fout,
                " <project>%s</project>\n",
                md.project->get_project_name()
            );
        }
        fprintf(fout, "</msg>\n");
    }
    fprintf(fout, "</msgs>\n");
}

int GUI_RPC_CONN::handle_rpc() {
    char buf[1024];
    int n;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_GUIRPC);

    // read the request message in one read()
	// so that the core client won't hang because
	// of malformed request msgs
	//
#ifdef _WIN32
        n = recv(sock, buf, 1024, 0);
#else
        n = read(sock, buf, 1024);
#endif
    if (n <= 0) return -1;
    buf[n] = 0;

    scope_messages.printf("GUI RPC Command = '%s'\n", buf);

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
	} else if (match_tag(buf, "<get_messages>")) {
		handle_get_messages(buf, fout);
    } else {
        fprintf(fout, "<unrecognized/>\n");
    }

#ifdef WIN32
    send(sock, fout->_base, strlen(fout->_base), NULL);
#else
    fflush(fout);
#endif

    return 0;
}

int GUI_RPC_CONN_SET::insert(GUI_RPC_CONN* p) {
    gui_rpcs.push_back(p);
    return 0;
}

int GUI_RPC_CONN_SET::init() {
	sockaddr_in addr;
    int retval;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_GUIRPC);

    lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0)
    {
        scope_messages.printf("GUI RPC failed to initialize socket (retval = '%d')\n", lsock);
#ifdef _WIN32
        scope_messages.printf("Windows Socket Error = '%d')\n", WSAGetLastError());
#endif
        return ERR_SOCKET;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    retval = bind(lsock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        scope_messages.printf("GUI RPC failed to bind to socket (retval = '%d')\n", retval);
#ifdef _WIN32
        scope_messages.printf("Windows Socket Error = '%d')\n", WSAGetLastError());
#endif
        return ERR_BIND;
    }
    retval = listen(lsock, 999);
    if (retval) {
        scope_messages.printf("GUI RPC failed to put socket into listening state (retval = '%d')\n", retval);
#ifdef _WIN32
        scope_messages.printf("Windows Socket Error = '%d')\n", WSAGetLastError());
#endif
        return ERR_LISTEN;
    }
    return 0;
}

bool GUI_RPC_CONN_SET::poll() {
    int n = 0;

    if (lsock >= 0) {
        unsigned int i;
        char buf[256];
        fd_set read_fds, error_fds;
        int sock, retval;
        vector<GUI_RPC_CONN*>::iterator iter;
        GUI_RPC_CONN* gr;
        struct timeval tv;

        sprintf(buf, "\r\nBOINC RPC Interface %d.%.2d\r\n\r\n", MAJOR_VERSION, MINOR_VERSION);

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
#ifdef _WIN32
            int addr_len = sizeof(addr);
            sock = accept(lsock, (struct sockaddr*)&addr, &addr_len);
#else
            size_t addr_len = sizeof(addr);
            sock = accept(lsock, (struct sockaddr*)&addr, &addr_len);
#endif
            int peer_ip = (int) ntohl(addr.sin_addr.s_addr);
            if ((!gstate.allow_remote_gui_rpc) && (peer_ip != 0x7f000001)) {
                msg_printf(
                    NULL, MSG_ERROR,
                    "GUI RPC request from non-local address 0x%x\n",
                    peer_ip
                );
            } else {
                GUI_RPC_CONN* gr = new GUI_RPC_CONN(sock);
                insert(gr);
#ifdef WIN32
                send(gr->sock, buf, strlen(buf), NULL);
#else
                write(gr->sock, buf, strlen(buf));
#endif
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
#ifdef WIN32
                send(gr->sock, buf, strlen(buf), NULL);
#else
                write(gr->sock, buf, strlen(buf));
#endif
            }
            iter++;
        }
    }
    return (n != 0);
}
