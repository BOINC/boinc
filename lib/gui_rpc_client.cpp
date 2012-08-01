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

// This file is the underlying mechanism of GUI RPC client
// (not the actual RPCs)

#include "gui_rpc_client.h"

#if defined(_WIN32) && !defined(__STDWX_H__) && !defined(_BOINC_WIN_) && !defined(_AFX_STDAFX_H_) 
#include "boinc_win.h"
#endif

#ifdef _WIN32
#include "../version.h"
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#else
#include "config.h"
#ifdef __EMX__
#include <sys/time.h>
#endif
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
#include "str_util.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "md5_file.h"
#include "network.h"
#include "common_defs.h"

using std::string;
using std::vector;

RPC_CLIENT::RPC_CLIENT() {
    sock = -1;
}

RPC_CLIENT::~RPC_CLIENT() {
    close();
}

#ifdef _WIN32
static int addr_len(sockaddr_storage&) {
    return (int) sizeof(sockaddr_in);
}
#else
static int addr_len(sockaddr_storage& s) {
    if (s.ss_family == AF_INET6) {
        return (int) sizeof(sockaddr_in6);
    }
    return (int) sizeof(sockaddr_in);
}
#endif

// if any RPC returns ERR_READ or ERR_WRITE,
// call this and then call init() again.
//
void RPC_CLIENT::close() {
    //fprintf(stderr, "RPC_CLIENT::close called\n");
	if (sock>=0) {
		boinc_close_socket(sock);
		sock = -1;
	}
}

int RPC_CLIENT::get_ip_addr(const char* host, int port) {
    memset(&addr, 0, sizeof(addr));
    //printf("trying port %d\n", htons(addr.sin_port));
    int retval;
    if (host) {
        retval = resolve_hostname(host, addr);
        if (retval) {
            return ERR_GETHOSTBYNAME;
        }
    } else {
        sockaddr_in* sin = (sockaddr_in*)&addr;
        sin->sin_family = AF_INET;
        sin->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }
    if (port) {
        port = htons(port);
    } else {
        port = htons(GUI_RPC_PORT);
    }
#ifdef _WIN32
    addr.sin_port = port;
#else
    if (addr.ss_family == AF_INET) {
        sockaddr_in* sin = (sockaddr_in*)&addr;
        sin->sin_port = port;
    } else {
        sockaddr_in6* sin = (sockaddr_in6*)&addr;
        sin->sin6_port = port;
    }
#endif
    return 0;
}

int RPC_CLIENT::init(const char* host, int port) {
    int retval = get_ip_addr(host, port);
    if (retval) return retval;
    boinc_socket(sock);

    // set up receive timeout; avoid hang if client doesn't respond
    //
#ifdef _WIN32
    DWORD dwTime = 30000;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&dwTime,  sizeof dwTime)) {
        // not fatal
    } 
#else
    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,  sizeof tv)) {
        // not fatal
    } 
#endif
    retval = connect(sock, (const sockaddr*)(&addr), addr_len(addr));
    if (retval) {
#ifdef _WIN32
        BOINCTRACE("RPC_CLIENT::init connect 2: Winsock error '%d'\n", WSAGetLastError());
#endif
        BOINCTRACE("RPC_CLIENT::init connect on %d returned %d\n", sock, retval);
        //perror("connect");
        close();
        return ERR_CONNECT;
    }

    return 0;
}

int RPC_CLIENT::init_asynch(
    const char* host, double _timeout, bool _retry, int port
) {
    int retval;
    retry = _retry;
    timeout = _timeout;

    retval = get_ip_addr(host, port);
    if (retval) return retval;

    retval = boinc_socket(sock);
    BOINCTRACE("init_asynch() boinc_socket: %d\n", sock);
    if (retval) return retval;

    retval = boinc_socket_asynch(sock, true);
    if (retval) {
        BOINCTRACE("init_asynch() boinc_socket_asynch: %d\n", retval);
    }
    start_time = dtime();
    retval = connect(sock, (const sockaddr*)(&addr), addr_len(addr));
    if (retval) {
        //perror("init_asynch(): connect");
        BOINCTRACE("init_asynch() connect: %d\n", retval);
    }
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

    BOINCTRACE("init_poll(): sock = %d\n", sock);

    tv.tv_sec = tv.tv_usec = 0;
    select(FD_SETSIZE, &read_fds, &write_fds, &error_fds, &tv);
    retval = 0;
    if (FD_ISSET(sock, &error_fds)) {
        retval =  ERR_CONNECT;
    } else if (FD_ISSET(sock, &write_fds)) {
        retval = get_socket_error(sock);
        if (!retval) {
            BOINCTRACE("init_poll(): connected\n");
            retval = boinc_socket_asynch(sock, false);
            if (retval) {
                BOINCTRACE("init_poll(): boinc_socket_asynch: %d\n", retval);
                return retval;
            }
            return 0;
        } else {
            BOINCTRACE("init_poll(): get_socket_error(): %d\n", retval);
        }
    }
    if (dtime() > start_time + timeout) {
        BOINCTRACE("asynch init timed out\n");
        return ERR_CONNECT;
    }
    if (retval) {
        if (retry) {
            boinc_close_socket(sock);
            retval = boinc_socket(sock);
            retval = boinc_socket_asynch(sock, true);
            retval = connect(sock, (const sockaddr*)(&addr), addr_len(addr));
            BOINCTRACE("init_poll(): retrying connect: %d\n", retval);
            return ERR_RETRY;
        } else {
            return ERR_CONNECT;
        }
    }
    return ERR_RETRY;
}

int RPC_CLIENT::authorize(const char* passwd) {
    bool found=false, authorized;
    int retval, n;
    char nonce[256], nonce_hash[256], buf[256];
    RPC rpc(this);
    XML_PARSER xp(&rpc.fin);

    retval = rpc.do_rpc("<auth1/>\n");
    if (retval) return retval;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.parse_str("nonce", nonce, sizeof(nonce))) {
            found = true;
            break;
        }
    }

    free(rpc.mbuf);
    rpc.mbuf = 0;

    if (!found) {
        //fprintf(stderr, "Nonce not found\n");
        return ERR_AUTHENTICATOR;
    }

    n = snprintf(buf, sizeof(buf), "%s%s", nonce, passwd);
    if (n >= (int)sizeof(buf)) return ERR_AUTHENTICATOR;
    md5_block((const unsigned char*)buf, (int)strlen(buf), nonce_hash);
    sprintf(buf, "<auth2>\n<nonce_hash>%s</nonce_hash>\n</auth2>\n", nonce_hash);
    retval = rpc.do_rpc(buf);
    if (retval) return retval;
    while (!xp.get_tag()) {
        if (!xp.is_tag) continue;
        if (xp.parse_bool("authorized", authorized)) {
            if (authorized) return 0;
            break;
        }
    }
    return ERR_AUTHENTICATOR;
}

int RPC_CLIENT::send_request(const char* p) {
    char buf[100000];
    sprintf(buf,
        "<boinc_gui_rpc_request>\n"
        "%s"
        "</boinc_gui_rpc_request>\n\003",
        p
    );
    int n = send(sock, buf, (int)strlen(buf), 0);
    if (n < 0) {
        //printf("send: %d\n", n);
        //perror("send");
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

RPC::RPC(RPC_CLIENT* rc) : xp(&fin) {
    mbuf = 0;
    rpc_client = rc;
}

RPC::~RPC() {
    if (mbuf) free(mbuf);
}

int RPC::do_rpc(const char* req) {
    int retval;

    //fprintf(stderr, "RPC::do_rpc rpc_client->sock = '%d'", rpc_client->sock);
    if (rpc_client->sock == -1) return ERR_CONNECT;
#ifdef DEBUG
    puts(req);
#endif
    retval = rpc_client->send_request(req);
    if (retval) return retval;
    retval = rpc_client->get_reply(mbuf);
    if (retval) return retval;
    fin.init_buf_read(mbuf);
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

// If there's a password file, read it
//
int read_gui_rpc_password(char* buf) {
    FILE* f = fopen(GUI_RPC_PASSWD_FILE, "r");
    if (!f) return ERR_FOPEN;
    char* p = fgets(buf, 256, f);
    if (p) {
        // trim CR
        //
        int n = (int)strlen(buf);
        if (n && buf[n-1]=='\n') {
            buf[n-1] = 0;
        }
    }
    fclose(f);
    return 0;
}

