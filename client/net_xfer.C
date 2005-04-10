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
#ifndef _CONSOLE
#include "stdafx.h"
#include "wingui_mainwindow.h"
#else
#include "boinc_win.h"
#endif

#include "win_util.h"

#endif

#ifndef _WIN32
#include <cstdio>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#endif

#include "error_numbers.h"
#include "net_xfer.h"
#include "util.h"
#include "network.h"

#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"

using std::vector;

// if an active transfer doesn't get any activity
// in this many seconds, error out
#define NET_XFER_TIMEOUT    600


// Attempt to open a nonblocking socket to a server
//
int NET_XFER::open_server() {
    sockaddr_in addr;
    int fd=0, ipaddr, retval=0;
    char msg[256];

#ifdef WIN32
    retval = NetOpen();
    if (retval) return retval;
#endif

    retval = resolve_hostname(hostname, ipaddr, msg);
    if (retval) {
        msg_printf(0, MSG_ERROR, "%s\n", msg);
        return retval;
    }

    retval = boinc_socket(fd);
    if (retval) return retval;
    retval = boinc_socket_asynch(fd, true);
    if (retval) return retval;

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
    NetClose();
#endif
    if (socket) {
        boinc_close_socket(socket);
        socket = 0;
    }
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
    xfer_speed = 0;
    reset_timeout();
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
// or .5 second goes by
//
bool NET_XFER_SET::poll(double now) {
    double bytes_xferred;
    int retval;
    bool action = false;

    while (1) {
        retval = do_select(bytes_xferred, 0);
        if (retval) break;
        if (bytes_xferred == 0) break;
        action = true;
        if ((dtime() - now) > 0.5) break;
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
        return poll(dtime());
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
                    nxp->update_speed();
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

    nbytes_transferred = 0;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    if (want_download) {
#ifdef _WIN32
        n = recv(socket, buf, blocksize, 0);
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
        // If we've sent the current contents of the buffer,
        // read the next block
        //
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
            n = send(socket, file_read_buf+file_read_buf_offset, nleft, 0);
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
void NET_XFER::update_speed() {
    double delta_t = dtime() - start_time;
    if (delta_t > 0) {
        xfer_speed = bytes_xferred / delta_t;
    } else if (xfer_speed == 0) {
        xfer_speed = 999999999;
    }
}

void NET_XFER::got_error() {
    //
    error = ERR_IO;
    io_done = true;
    log_messages.printf(
        CLIENT_MSG_LOG::DEBUG_NET_XFER, "IO error on socket %d\n", socket
    );
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

const char *BOINC_RCSID_e0a7088e04 = "$Id$";
