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

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"

#ifndef _CONSOLE
#include "wingui_mainwindow.h"
#endif

#include "Win_net.h"
#include "win_util.h"

#endif

#ifndef _WIN32

#include <stdio.h>
#include <math.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#endif

#include "error_numbers.h"
#include "net_xfer.h"
#include "util.h"
#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"

#if defined(_WIN32) 
typedef int socklen_t;
#elif defined ( __APPLE__)
typedef int32_t socklen_t;
#elif !GETSOCKOPT_SOCKLEN_T
#ifndef socklen_t
typedef size_t socklen_t;
#endif
#endif

// if an active transfer doesn't get any activity
// in this many seconds, error out
#define NET_XFER_TIMEOUT    600

int get_socket_error(int fd) {
    socklen_t intsize = sizeof(int);
    int n;
#ifdef WIN32
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, &intsize);
#elif __APPLE__
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, (int *)&intsize);
#else
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, &intsize);
#endif
    return n;
}

NET_XFER::NET_XFER(){
    ssl = NULL;
    ctx = NULL;
    url_mode = 0;
    proxy_connect_finished = false;
}

NET_XFER::~NET_XFER() {
}

int NET_XFER::get_ip_addr(int &ip_addr) {
    return get_ip_addr(hostname, ip_addr);
}

int NET_XFER::get_ip_addr(char *hostname, int &ip_addr) {
    hostent* hep;

#ifdef WIN32
    int retval;
    retval = NetOpen();
    if (retval) return retval;
#endif
    hep = gethostbyname(hostname);
    if (!hep) {
        char msg[256];
        int n;

        n = sprintf(msg, "Can't resolve hostname %s ", hostname);
#ifdef WIN32

        switch (WSAGetLastError()) {
        case WSANOTINITIALISED:
            break;
        case WSAENETDOWN:
            sprintf(msg+n, "(the network subsystem has failed)");
            break;
        case WSAHOST_NOT_FOUND:
            sprintf(msg+n, "(host name not found)");
            break;
        case WSATRY_AGAIN:
            sprintf(msg+n, "(no response from server)");
            break;
        case WSANO_RECOVERY:
            sprintf(msg+n, "(a nonrecoverable error occurred)");
            break;
        case WSANO_DATA:
            sprintf(msg+n, "(valid name, no data record of requested type)");
            break;
        case WSAEINPROGRESS:
            sprintf(msg+n, "(a blocking socket call in progress)");
            break;
        case WSAEFAULT:
            sprintf(msg+n, "(invalid part of user address space)");
            break;
        case WSAEINTR:
            sprintf(msg+n, "(a blocking socket call was canceled)");
            break;
        }
        NetClose();

#else

        switch (h_errno) {
        case HOST_NOT_FOUND:
            sprintf(msg+n, "(host not found)");
            break;
        case NO_DATA:
            sprintf(msg+n, "(valid name, no data record of requested type)");
            break;
        case NO_RECOVERY:
            sprintf(msg+n, "(a nonrecoverable error occurred)");
            break;
        case TRY_AGAIN:
            sprintf(msg+n, "(host not found or server failure)");
            break;
        }

#endif
        msg_printf(0, MSG_ERROR, "%s\n", msg);
        return ERR_GETHOSTBYNAME;
    }
    ip_addr = *(int*)hep->h_addr_list[0];

    return 0;
}


// Attempt to open a nonblocking socket to a server
//
int NET_XFER::open_server() {
    sockaddr_in addr;
    int fd=0, ipaddr, retval=0;

    retval = get_ip_addr(hostname, ipaddr);
    if (retval) return retval;

    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
#ifdef WIN32
        NetClose();
#endif
        return ERR_SOCKET;
    }

#ifdef WIN32
    unsigned long one = 1;
    ioctlsocket(fd, FIONBIO, &one);
#else
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        return ERR_FCNTL;
    }
    if (fcntl(fd, F_SETFL, flags|O_NONBLOCK) < 0 ) {
        return ERR_FCNTL;
    }
#endif

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ((long)ipaddr);
    retval = connect(fd, (sockaddr*)&addr, sizeof(addr));
    if (retval) {
#ifdef WIN32
        errno = WSAGetLastError();
        if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK) {
            closesocket(fd);
            NetClose();
            return ERR_CONNECT;
        }
#ifndef _CONSOLE
        if (WSAAsyncSelect( fd, g_myWnd->GetSafeHwnd(), RegisterWindowMessage(NET_ACTIVITY_MSG), FD_READ|FD_WRITE )) {
            errno = WSAGetLastError();
            if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK) {
                closesocket(fd);
                NetClose();
                return ERR_ASYNCSELECT;
            }
        }
#endif
#else
        if (errno != EINPROGRESS) {
            close(fd);
            perror("connect");
            return ERR_CONNECT;
        }
#endif
    } else {
        is_connected = true;
    }
    socket = fd;
    return 0;
}

void NET_XFER::close_socket() {
#ifdef WIN32
    close_ssl();
    NetClose();
    if (socket) closesocket(socket);
#else
    if (socket) close(socket);
#endif
}

void NET_XFER::init(char* host, int p, int b) {
    socket = -1;
    is_connected = false;
    want_download = false;
    want_upload = false;
    do_file_io = false;
    io_done = false;
    file = NULL;
    io_ready = false;
    error = 0;
    safe_strcpy(hostname, host);
    port = p;
    blocksize = (b > MAX_BLOCKSIZE ? MAX_BLOCKSIZE : b);
    start_time = dtime();
    file_read_buf_offset = 0;
    file_read_buf_len = 0;
    bytes_xferred = 0;
    xfer_speed = -1;
    reset_timeout();
}

void NET_XFER::init_2() {
    is_connected = false;
    want_download = false;
    want_upload = false;
    do_file_io = false;
    io_done = false;
    file = NULL;
    io_ready = false;
    error = 0;
    xfer_speed = 0;
    file_read_buf_offset = 0;
    file_read_buf_len = 0;
    bytes_xferred = 0;
}

bool NET_XFER::check_timeout(bool time_passed) {
    if (seconds_until_timeout == 0) {
        io_done = true;
        error = ERR_TIMEOUT;
        return true;
    }
    if (time_passed) {
        seconds_until_timeout--;
    }
    return false;
}

void NET_XFER::reset_timeout() {
    seconds_until_timeout = NET_XFER_TIMEOUT;
}

char* NET_XFER::get_hostname() {
    return hostname;
}

NET_XFER_SET::NET_XFER_SET() {
    max_bytes_sec_up = 0;
    max_bytes_sec_down = 0;
    bytes_left_up = 0;
    bytes_left_down = 0;
    bytes_up = 0;
    bytes_down = 0;
    up_active = false;
    down_active = false;
}

// Insert a NET_XFER object into the set
//
int NET_XFER_SET::insert(NET_XFER* nxp) {
    int retval = nxp->open_server();
    if (retval) return retval;
    net_xfers.push_back(nxp);
    return 0;
}

// Remove a NET_XFER object from the set
//
int NET_XFER_SET::remove(NET_XFER* nxp) {
    vector<NET_XFER*>::iterator iter;

    nxp->close_socket();

    iter = net_xfers.begin();
    while (iter != net_xfers.end()) {
        if (*iter == nxp) {
            net_xfers.erase(iter);
            return 0;
        }
        iter++;
    }
    msg_printf(NULL, MSG_ERROR, "NET_XFER_SET::remove(): not found\n");
    return ERR_NOT_FOUND;
}

// Transfer data to/from active sockets.
// Keep doing I/O until would block, or we hit rate limits,
// or about .5 second goes by
//
bool NET_XFER_SET::poll() {
    double bytes_xferred;
    int retval;
    time_t t = time(0);
    bool action = false;

    while (1) {
        retval = do_select(bytes_xferred, 0);
        if (retval) break;
        if (bytes_xferred == 0) break;
        action = true;
        if (time(0) != t) break;
    }
    return action;
}

static void double_to_timeval(double x, timeval& t) {
    t.tv_sec = (int)x;
    t.tv_usec = (int)(1000000*(x - (int)x));
}

// Wait at most x seconds for network I/O to become possible,
// then do up to about .5 seconds of I/O.
//
int NET_XFER_SET::net_sleep(double x) {
    int retval;
    double bytes_xferred;

    retval = do_select(bytes_xferred, x);
    if (retval) return retval;
    if (bytes_xferred) {
        return poll();
    }
    return 0;
}

// do a select with the given timeout,
// then do I/O on as many sockets as possible, subject to rate limits
// Transfer at most one block per socket.
//
int NET_XFER_SET::do_select(double& bytes_transferred, double timeout) {
    int n, fd, retval, nsocks_queried;
    unsigned int i;
    NET_XFER *nxp;
    struct timeval tv;
    bool time_passed = false;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    // if a second has gone by, do rate-limit accounting
    //
    time_t t = time(0);
    if (t != last_time) {
        time_passed = true;
        last_time = t;
        if (bytes_left_up < max_bytes_sec_up) {
            bytes_left_up += max_bytes_sec_up;
        }
        if (bytes_left_down < max_bytes_sec_down) {
            bytes_left_down += max_bytes_sec_down;
        }
    }

    bytes_transferred = 0;

    fd_set read_fds, write_fds, error_fds;

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    // do a select on all active (non-throttled) sockets
    //
    nsocks_queried = 0;
    for (i=0; i<net_xfers.size(); i++) {
        nxp = net_xfers[i];
        if (!nxp->is_connected) {
            if (nxp->check_timeout(time_passed)) continue;
            FD_SET(nxp->socket, &write_fds);
            nsocks_queried++;
        } else if (nxp->want_download) {
            if (nxp->check_timeout(time_passed)) continue;
            if (bytes_left_down > 0) {
                FD_SET(nxp->socket, &read_fds);
                nsocks_queried++;
            } else {
                scope_messages.printf("NET_XFER_SET::do_select(): Throttling download\n");
            }
        } else if (nxp->want_upload) {
            if (nxp->check_timeout(time_passed)) continue;
            if (bytes_left_up > 0) {
                FD_SET(nxp->socket, &write_fds);
                nsocks_queried++;
            } else {
                scope_messages.printf("NET_XFER_SET::do_select(): Throttling upload\n");
            }
        }
        FD_SET(nxp->socket, &error_fds);
    }
    if (nsocks_queried==0) {
        boinc_sleep(timeout);
        return 0;
    }

    double_to_timeval(timeout, tv);
    n = select(FD_SETSIZE, &read_fds, &write_fds, &error_fds, &tv);
    scope_messages.printf(
        "NET_XFER_SET::do_select(): queried %d, returned %d\n",
        nsocks_queried, n
    );
    if (n == 0) return 0;
    if (n < 0) return ERR_SELECT;

    // if got a descriptor, find the first one in round-robin order
    // and do I/O on it
    // TODO: use round-robin order
    //
    for (i=0; i<net_xfers.size(); i++) {
        nxp = net_xfers[i];
        fd = nxp->socket;
        if (FD_ISSET(fd, &read_fds) || FD_ISSET(fd, &write_fds)) {
            if (FD_ISSET(fd, &read_fds)) {
                scope_messages.printf("NET_XFER_SET::do_select(): read enabled on socket %d\n", fd);
            }
            if (FD_ISSET(fd, &write_fds)) {
                scope_messages.printf("NET_XFER_SET::do_select(): write enabled on socket %d\n", fd);
            }
            if (!nxp->is_connected) {
                n = get_socket_error(fd);
                if (n) {
                    scope_messages.printf(
                        "NET_XFER_SET::do_select(): socket %d connection to %s failed\n",
                        fd, nxp->get_hostname()
                    );
                    nxp->error = ERR_CONNECT;
                    nxp->io_done = true;
                } else {
                    scope_messages.printf("NET_XFER_SET::do_select(): socket %d is connected\n", fd);
                    nxp->is_connected = true;
                    bytes_transferred += 1;
                    nxp->reset_timeout();
                }
            } else if (nxp->do_file_io) {
                n = 1;
                time_t now = time(0);
                do {
                    retval = nxp->do_xfer(n);
                    nxp->update_speed(n);
                    nxp->reset_timeout();
                    bytes_transferred += n;
                    if (nxp->want_download) {
                        down_active = true;
                        bytes_left_down -= n;
                        bytes_down += n;
                    } else {
                        up_active = true;
                        bytes_left_up -= n;
                        bytes_up += n;
                    }
                    // For uploads, keep trying to send until we fill
                    // the buffers or 1 second has passed
                } while(nxp->want_upload && n > 0 && time(0) == now);
            } else {
                nxp->io_ready = true;
            }
        } else if (FD_ISSET(fd, &error_fds)) {
            scope_messages.printf("NET_XFER_SET::do_select(): got error on socket %d\n", fd);
            nxp = lookup_fd(fd);
            if (nxp) {
                nxp->got_error();
            } else {
                msg_printf(0, MSG_ERROR, "do_select(): nxp not found\n");
            }
        }
    }
    return 0;
}

// Return the NET_XFER object whose socket matches fd
//
NET_XFER* NET_XFER_SET::lookup_fd(int fd) {
    for (unsigned int i=0; i<net_xfers.size(); i++) {
        if (net_xfers[i]->socket == fd) {
            return net_xfers[i];
        }
    }
    return 0;
}

// transfer up to a block of data; return #bytes transferred
//
int NET_XFER::do_xfer(int& nbytes_transferred) {
    // Leave these as signed ints so recv/send can return errors
    int n, m, nleft;
    bool would_block;
    char buf[MAX_BLOCKSIZE];
	int ssl_ret=0;

    nbytes_transferred = 0;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    if (want_download) {
#ifdef _WIN32
        if(url_mode == URL_IS_SSL){
            memset(buf,0,sizeof(buf));
            init_ssl();
            n = ssl_connect(socket,&ssl_ret);
            ssl_error_print("NET_XFER::do_xfer[want_download]", n, ssl_ret);
            n = ssl_read_write(socket, buf, SSL_READ_MODE, 0, &ssl_ret);
            if(n < 0){
                n = 0;
            }
            ssl_error_print("NET_XFER::do_xfer[want_download]", n, ssl_ret);
        }else{
            n = recv(socket, buf, blocksize, 0);
        }
#else
        n = read(socket, buf, blocksize);
#endif
        scope_messages.printf("NET_XFER::do_xfer(): read %d bytes from socket %d\n", n, socket);
        if (n == 0) {
            io_done = true;
            want_download = false;
        } else if (n < 0) {
            io_done = true;
            error = ERR_READ;
        } else {
            nbytes_transferred += n;
            bytes_xferred += n;
            m = fwrite(buf, 1, n, file);
            if (n != m) {
                fprintf(stdout, "Error: incomplete disk write\n");
                io_done = true;
                error = ERR_FWRITE;
            }
        }
    } else if (want_upload) {
        // If we've sent the current contents of
        // the buffer, then read the next block
        if (file_read_buf_len == file_read_buf_offset) {
            m = fread(file_read_buf, 1, blocksize, file);
            if (m == 0) {
                want_upload = false;
                io_done = true;
                return 0;
            } else if (m < 0) {
                io_done = true;
                error = ERR_FREAD;
                return 0;
            }
            file_read_buf_len = m;
            file_read_buf_offset = 0;
        }
        nleft = file_read_buf_len - file_read_buf_offset;
        while (nleft) {
#ifdef WIN32
            if(url_mode == URL_IS_SSL){
                init_ssl();
                n = ssl_connect(socket, &ssl_ret);
                ssl_error_print("NET_XFER::do_xfer[want_upload]", n, ssl_ret);
                n = ssl_read_write(socket, file_read_buf+file_read_buf_offset, SSL_WRITE_MODE, nleft, &ssl_ret);
                ssl_error_print("NET_XFER::do_xfer[want_upload]", n, ssl_ret);
            }else{
                n = send(socket, file_read_buf+file_read_buf_offset, nleft, 0);
            }
            would_block = (WSAGetLastError() == WSAEWOULDBLOCK);
#else
            n = write(socket, file_read_buf+file_read_buf_offset, nleft);
            would_block = (errno == EAGAIN);
#endif
            if (would_block && n < 0) n = 0;
            scope_messages.printf(
                "NET_XFER::do_xfer(): wrote %d bytes to socket %d%s\n",
                n, socket, (would_block?", would have blocked":"")
            );
            if (n < 0 && !would_block) {
                error = ERR_WRITE;
                io_done = true;
                break;
            }

            file_read_buf_offset += n;
            nbytes_transferred += n;
            bytes_xferred += n;

            if (n < nleft || would_block) {
                break;
            }

            nleft -= n;
        }
    }
    return 0;
}

// Update the transfer speed for this NET_XFER
// called on every I/O
//
void NET_XFER::update_speed(int nbytes) {
    double delta_t = dtime() - start_time;
    if (delta_t > 0) {
        xfer_speed = bytes_xferred / delta_t;
    } else if (xfer_speed == 0) {
        xfer_speed = 999999999;
    }
}

void NET_XFER::got_error() {
    error = ERR_IO;
    io_done = true;
    log_messages.printf(
        CLIENT_MSG_LOG::DEBUG_NET_XFER, "IO error on socket %d\n", socket
    );
}

void NET_XFER::init_ssl()
{
    static bool init_on = false;
    if(!init_on){
        SSL_library_init();
        SSL_load_error_strings();
        init_on = true;
    }
}

int NET_XFER::close_ssl()
{
    int rc;
    if(ssl != NULL){
        rc = SSL_shutdown(ssl);
        if (rc < 0) {
            ssl = NULL; // check!
            return -1;
        }
        if (rc == 0) {
            rc = SSL_shutdown(ssl);
        }
        // Free SSL Object
        SSL_free(ssl);
        ssl = NULL;
    }
    if(ctx != NULL){
        SSL_CTX_free(ctx);
        ctx = NULL;
    }
    return 0;
}

void NET_XFER::ssl_error_print(char *in_buf, int in_status, int inerrno)
{
    char msg_buf[256];
    memset(msg_buf,0,sizeof(msg_buf));
    switch (in_status) {
        case  0:
            break;
        case -1:
            sprintf(msg_buf, "%s : SSL ERROR : %d",in_buf, inerrno);
            break;
        case -2:
            sprintf(msg_buf, "%s : SSL CONNECT ERROR : %d",in_buf, inerrno);
            break;
    }
    if(in_status < 0){
        msg_printf(NULL, MSG_ERROR, "%s\n", msg_buf);
    }
}

int NET_XFER::ssl_connect(int insocket, int *insslerr)
{
    int rc;
    int ret=true;
    int ssl_error_code=0;

    *insslerr=0;
    if(ctx == NULL){
        ctx = SSL_CTX_new(SSLv23_client_method());
        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, insocket);
        rc = SSL_connect(ssl);
        if (rc <= 0) {
            ssl_error_code = SSL_get_error(ssl, rc);
            switch (ssl_error_code) {
                case SSL_ERROR_WANT_READ:
                    fprintf(stderr, "SSL_ERROR_WANT_READ\n");
                    ret = true;
                    break;
                case SSL_ERROR_WANT_WRITE:
                    fprintf(stderr, "SSL_ERROR_WANT_WRITE\n");
                    ret = true;
                    break;
                case SSL_ERROR_ZERO_RETURN:
                    fprintf(stderr, "SSL_ERROR_ZERO_RETRUN\n");
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
                case SSL_ERROR_WANT_CONNECT:
                    fprintf(stderr, "SSL_ERROR_WANT_CONNECT\n");
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
                case SSL_ERROR_WANT_X509_LOOKUP:
                    fprintf(stderr, "SSL_ERROR_WANT_X509_LOOKUP\n");
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
                case SSL_ERROR_SYSCALL:
                    fprintf(stderr, "SSL_ERROR_SYSCALL:%d\n", rc);
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
                case SSL_ERROR_SSL:
                    fprintf(stderr, "SSL_ERROR_SSL\n");
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
                default:
                    *insslerr = ssl_error_code;
                    ret = -2;
                    break;
            }
        }
    }
    return ret;
}

int NET_XFER::ssl_read_write(int insocket, char *inbuf, int r_w, int inwtsize, int *insslerr)
{
    struct timeval tv;
    fd_set readfds, writefds;
    int read_blocked_on_write = 0;
    int write_blocked_on_read = 0;
    int read_blocked = 0;
    int sel;
    int writehook = 0;
    char rbuf[MAX_BLOCKSIZE];
    int wlen = 0;
    int wlenplus = 0;
    int rc;
    int ssl_error_code = 0;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // Read & Write loop
    while (1) {
        *insslerr=0;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        writehook = 0;
        memset(rbuf,0,sizeof(rbuf));

        FD_SET(insocket, &readfds);

        if( r_w == SSL_WRITE_MODE ){
            wlen = inwtsize;
        }

        if (!write_blocked_on_read) {
            if (wlen || read_blocked_on_write) {
                FD_SET(insocket, &writefds);
            }else {
                writehook = 1;
            }
        }

        sel = select(insocket + 1, &readfds, &writefds, 0, &tv);
        if (sel == 0) {
            if (writehook == 0) {
                continue;
            }
        }

        // Readable ?
        if ((FD_ISSET(insocket, &readfds) && !write_blocked_on_read)
         || (read_blocked_on_write && FD_ISSET(insocket, &writefds))) {
            do {
                read_blocked_on_write = 0;
                read_blocked = 0;

                rc = SSL_read(ssl, rbuf, MAX_BLOCKSIZE);
                ssl_error_code = SSL_get_error(ssl, rc);
                switch(ssl_error_code) {
                    case SSL_ERROR_NONE:
                        if( r_w == SSL_READ_MODE ){
                            memcpy(inbuf,rbuf,rc);
                            return rc;
                        }
                        break;
                    case SSL_ERROR_ZERO_RETURN:
                        return 0;
                        break;
                    case SSL_ERROR_WANT_READ:
                        read_blocked = 1;
                        break;
                    case SSL_ERROR_WANT_WRITE:
                        read_blocked_on_write = 1;
                        break;
                    default:
                        *insslerr = ssl_error_code;
                        return -1;
                        break;
                }
            } while (SSL_pending(ssl) && !read_blocked);
        }

        // Writeable ?
        if ((FD_ISSET(insocket, &writefds) && wlen > 0)
         || (write_blocked_on_read && FD_ISSET(insocket, &readfds))) {
            write_blocked_on_read = 0;

            rc = SSL_write(ssl, inbuf, wlen);

            ssl_error_code = SSL_get_error(ssl, rc);
            switch(ssl_error_code) {
                case SSL_ERROR_NONE:
                    if( r_w == SSL_WRITE_MODE ){
                        wlen -= rc;
                        inbuf += rc;
                        wlenplus += rc;
                        if(wlen == 0){
                            return wlenplus;
                        }
                    }
                    break;
                case SSL_ERROR_WANT_WRITE:
                    break;
                case SSL_ERROR_WANT_READ:
                    write_blocked_on_read = 1;
                    break;
                default:
                    *insslerr = ssl_error_code;
                    return -1;
            }
        }
    }
}

// return true if an upload is currently in progress
// or has been since the last call to this.
// Similar for download.
//
void NET_XFER_SET::check_active(bool& up, bool& down) {
    unsigned int i;
    NET_XFER* nxp;

    up = up_active;
    down = down_active;
    for (i=0; i<net_xfers.size(); i++) {
        nxp = net_xfers[i];
        if (nxp->is_connected && nxp->do_file_io) {
            nxp->want_download?down=true:up=true;
        }
    }
    up_active = false;
    down_active = false;
}

const char *BOINC_RCSID_7fd7712781 = "$Id$";
