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

// This file is the underlying mechanism of GUI RPC client
// (not the actual RPCs)

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_)
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "../version.h"
#else
#include "config.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#endif

#include "diagnostics.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "gui_rpc_client.h"

using std::string;
using std::vector;

RPC_CLIENT::RPC_CLIENT() {}

RPC_CLIENT::~RPC_CLIENT() {
    close();
}

// if any RPC returns ERR_READ or ERR_WRITE,
// call this and then call init() again.
//
void RPC_CLIENT::close() {
    //fprintf(stderr, "RPC_CLIENT::close called\n");
	if (sock) {
		boinc_close_socket(sock);
		sock = 0;
	}
}

int RPC_CLIENT::init(const char* host) {
    int retval;
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT_ALT);

    if (host) {
        hostent* hep = gethostbyname(host);
        if (!hep) {
            perror("gethostbyname");
            return ERR_GETHOSTBYNAME;
        }
        addr.sin_addr.s_addr = *(int*)hep->h_addr_list[0];
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }
    boinc_socket(sock);
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        BOINCTRACE("RPC_CLIENT::init connect 2 on %d returned %d\n", sock, retval);
        //perror("connect");
        boinc_close_socket(sock);
        boinc_socket(sock);
#ifdef _WIN32
        BOINCTRACE("RPC_CLIENT::init connect 1: Winsock error '%d'\n", WSAGetLastError());
#endif
        addr.sin_port = htons(GUI_RPC_PORT);
        retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
        if (retval) {
#ifdef _WIN32
            BOINCTRACE("RPC_CLIENT::init connect 2: Winsock error '%d'\n", WSAGetLastError());
#endif
            BOINCTRACE("RPC_CLIENT::init connect on %d returned %d\n", sock, retval);
            perror("connect");
            close();
            return ERR_CONNECT;
        }
    }
    return 0;
}

int RPC_CLIENT::init_asynch(const char* host, double _timeout, bool _retry) {
    int retval;
	memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(GUI_RPC_PORT_ALT);
    retry = _retry;
    timeout = _timeout;

    if (host) {
        hostent* hep = gethostbyname(host);
        if (!hep) {
            perror("gethostbyname");
            return ERR_GETHOSTBYNAME;
        }
        addr.sin_addr.s_addr = *(int*)hep->h_addr_list[0];
    } else {
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    retval = boinc_socket(sock);
    BOINCTRACE("RPC_CLIENT::init boinc_socket returned %d\n", sock);
    if (retval) return retval;

    tried_alt_port = false;
    retval = boinc_socket_asynch(sock, true);
    if (retval) {
        BOINCTRACE("RPC_CLIENT::init asynch error: %d\n", retval);
    }
    start_time = dtime();
    retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
    if (retval) {
        perror("connect");
        BOINCTRACE("RPC_CLIENT::init connect returned %d\n", retval);
    }
    BOINCTRACE("RPC_CLIENT::init attempting connect to alt port\n");
    return 0;
}

int RPC_CLIENT::init_poll() {
    fd_set read_fds, write_fds, error_fds;
    struct timeval tv;
    int retval;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    FD_SET(sock, &read_fds);
    FD_SET(sock, &write_fds);
    FD_SET(sock, &error_fds);

    BOINCTRACE("RPC_CLIENT::init_poll sock = %d\n", sock);

    tv.tv_sec = tv.tv_usec = 0;
    select(FD_SETSIZE, &read_fds, &write_fds, &error_fds, &tv);
    retval = 0;
    if (FD_ISSET(sock, &error_fds)) {
        retval =  ERR_CONNECT;
    } else if (FD_ISSET(sock, &write_fds)) {
        retval = get_socket_error(sock);
        if (!retval) {
            BOINCTRACE("RPC_CLIENT::init_poll connected to port %d\n", ntohs(addr.sin_port));
            retval = boinc_socket_asynch(sock, false);
            if (retval) {
                fprintf(stderr, "asynch error: %d\n", retval);
                return retval;
            }
            return 0;
        } else {
            fprintf(stderr, "init_poll: get_socket_error(): %d\n", retval);
        }
    }
    if (dtime() > start_time + timeout) {
        fprintf(stderr, "RPC_CLIENT init timed out\n");
        return ERR_CONNECT;
    }
    if (retval) {
        if (!retry && tried_alt_port) {
            fprintf(stderr, "already tried both ports, giving up\n");
            return ERR_CONNECT;
        } else {
            boinc_close_socket(sock);
            retval = boinc_socket(sock);
            retval = boinc_socket_asynch(sock, true);
            addr.sin_port = htons(tried_alt_port?GUI_RPC_PORT_ALT:GUI_RPC_PORT);
            retval = connect(sock, (const sockaddr*)(&addr), sizeof(addr));
            BOINCTRACE(
                "RPC_CLIENT::init_poll attempting connect to %s port\n",
                tried_alt_port?"alternate":"main"
            );
            tried_alt_port = !tried_alt_port;
            return ERR_RETRY;
        }
    }
    return ERR_RETRY;
}

int RPC_CLIENT::authorize(const char* passwd) {
    bool found=false;
    int retval;
    char buf[256], nonce[256], nonce_hash[256];
    RPC rpc(this);

    retval = rpc.do_rpc("<auth1/>\n");
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (parse_str(buf, "<nonce>", nonce, sizeof(nonce))) {
            found = true;
        }
    }
    if (!found) {
        fprintf(stderr, "Nonce not found\n");
        return ERR_AUTHENTICATOR;
    }

    sprintf(buf, "%s%s", nonce, passwd);
    md5_block((const unsigned char*)buf, strlen(buf), nonce_hash);
    sprintf(buf, "<auth2/>\n<nonce_hash>%s</nonce_hash>\n", nonce_hash);
    retval = rpc.do_rpc(buf);
    if (retval) return retval;
    while (rpc.fin.fgets(buf, 256)) {
        if (match_tag(buf, "<authorized/>")) {
            return 0;
        }
    }
    return ERR_AUTHENTICATOR;
}

int RPC_CLIENT::send_request(const char* p) {
    char buf[4096];
    sprintf(buf,
        "<boinc_gui_rpc_request>\n"
        "   <major_version>%d</major_version>\n"
        "   <minor_version>%d</minor_version>\n"
        "   <release>%d</release>\n"
        "%s"
        "</boinc_gui_rpc_request>\n\003",
        BOINC_MAJOR_VERSION,
        BOINC_MINOR_VERSION,
        BOINC_RELEASE,
        p
    );
    int n = send(sock, buf, strlen(buf), 0);
    if (n < 0) {
        printf("send: %d\n", n);
        perror("send");
        return ERR_WRITE;
    }
    return 0;
}

// get reply from server.  Caller must free buf
//
int RPC_CLIENT::get_reply(char*& mbuf) {
    char buf[8193];
    MFILE mf;
    int n;

    while (1) {
        n = recv(sock, buf, 8192, 0);
        if (n <= 0) return ERR_READ;
        buf[n]=0;
        mf.puts(buf);
        if (strchr(buf, '\003')) break;
    }
    mf.get_buf(mbuf, n);
    return 0;
}

RPC::RPC(RPC_CLIENT* rc) {
    mbuf = 0;
    rpc_client = rc;
}

RPC::~RPC() {
    if (mbuf) free(mbuf);
}

int RPC::do_rpc(const char* req) {
    int retval;

    //fprintf(stderr, "RPC::do_rpc rpc_client->sock = '%d'", rpc_client->sock);
    if (rpc_client->sock == 0) return ERR_CONNECT;
#ifdef DEBUG
    puts(req);
#endif
    retval = rpc_client->send_request(req);
    if (retval) return retval;
    retval = rpc_client->get_reply(mbuf);
    if (retval) return retval;
    fin.init_buf(mbuf);
#ifdef DEBUG
    puts(mbuf);
#endif
    return 0;
}

int RPC::parse_reply() {
    char buf[256];
    while (fin.fgets(buf, 256)) {
        if (strstr(buf, "unauthorized")) return ERR_AUTHENTICATOR;
        if (strstr(buf, "Missing authenticator")) return ERR_AUTHENTICATOR;
        if (strstr(buf, "Missing URL")) return ERR_INVALID_URL;
        if (strstr(buf, "Already attached to project")) return ERR_ALREADY_ATTACHED;
        if (strstr(buf, "success")) return BOINC_SUCCESS;
    }
    return ERR_NOT_FOUND;
}

const char *BOINC_RCSID_6802bead97 = "$Id$";
