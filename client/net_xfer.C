// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
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

#include "windows_cpp.h"

#include <stdio.h>

#ifdef _WIN32
#include "winsock.h"
#endif

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
//#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "error_numbers.h"
#include "log_flags.h"
#include "net_xfer.h"

// On Macs, socklen_t isn't defined in a header file so we have to define it here
#ifdef mac
#define socklen_t unsigned int
#endif
//  The other option is that the socket functions require size_t instead
#if !defined(socklen_t)
#define socklen_t size_t
#endif

// Attempt to open a nonblocking socket to a server
//
int NET_XFER::open_server() {
    sockaddr_in addr;
    hostent* hep;
    int fd=0, ipaddr, retval=0;

    hep = gethostbyname(hostname);
    if (!hep) {
        fprintf(stderr, "can't resolve hostname %s\n", hostname);
        return ERR_GETHOSTBYNAME;
    }
    ipaddr = *(int*)hep->h_addr_list[0];
    if (retval) return -1;
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

#ifdef _WIN32
    unsigned long one = 1;
    ioctlsocket(fd, FIONBIO, &one);
#else
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    else if (fcntl(fd, F_SETFL, flags|O_NONBLOCK) < 0 ) return -1;
#endif

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = ((long)ipaddr);
    retval = connect(fd, (sockaddr*)&addr, sizeof(addr));
    if (retval) {
#ifdef _WIN32
        errno = WSAGetLastError();
        if (errno != WSAEINPROGRESS && errno != WSAEWOULDBLOCK) {
            closesocket(fd);
            return -1;
        }
#else
        if (errno != EINPROGRESS) {
            close(fd);
            perror("connect");
            return -1;
        }
#endif
    } else {
        is_connected = true;
    }
    socket = fd;
    return 0;
}

void NET_XFER::close_socket() {
#ifdef _WIN32
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
    strcpy(hostname, host);
    port = p;
    blocksize = b;
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
    fprintf(stdout, "NET_XFER_SET::remove(): not found\n");
    return 1;
}

// transfer data to/from a list of active streams
// transfer at most max_bytes bytes.
// TODO: implement other bandwidth constraints (ul/dl ratio, time of day)
//
int NET_XFER_SET::poll(int max_bytes, int& bytes_transferred) {
    int n, retval;

    bytes_transferred = 0;
    while (1) {
        retval = do_select(max_bytes, n);
        if (retval) return retval;
        if (n == 0) break;
        max_bytes -= n;
        bytes_transferred += n;
        if (max_bytes < 0) break;
    }
    return 0;
}

// do a select and do I/O on as many sockets as possible.
// 
int NET_XFER_SET::do_select(int max_bytes, int& bytes_transferred) {
    struct timeval zeros;
    int n, fd, retval;
    socklen_t i;
    NET_XFER *nxp;
#if GETSOCKOPT_SIZE_T
    size_t intsize = sizeof(int);
#elif GETSOCKOPT_SOCKLEN_T
    socklen_t intsize = sizeof(int);
#else
    //int intsize = sizeof(int);
    socklen_t intsize = sizeof(int);
#endif

    bytes_transferred = 0;

    fd_set read_fds, write_fds, error_fds;
    memset(&zeros, 0, sizeof(zeros));

    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

    // do a select on all active streams
    //
    for (i=0; i<net_xfers.size(); i++) {
	nxp = net_xfers[i];
	if (!nxp->is_connected) {
	    FD_SET(net_xfers[i]->socket, &write_fds);
	} else if (net_xfers[i]->want_download) {
	    FD_SET(net_xfers[i]->socket, &read_fds);
	} else if (net_xfers[i]->want_upload) {
	    FD_SET(net_xfers[i]->socket, &write_fds);
	}
	FD_SET(net_xfers[i]->socket, &error_fds);
    }
    n = select(FD_SETSIZE, &read_fds, &write_fds, &error_fds, &zeros);
    if (log_flags.net_xfer_debug) printf("select returned %d\n", n);
    if (n == 0) return 0;
    if (n < 0) return ERR_SELECT;

    // if got a descriptor, find the first one in round-robin order
    // and do I/O on it
    for (i=0; i<net_xfers.size(); i++) {
	nxp = net_xfers[i];
        fd = nxp->socket;
	if (FD_ISSET(fd, &read_fds) || FD_ISSET(fd, &write_fds)) {
            if (!nxp->is_connected) {
#ifdef _WIN32
                getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&n, (int *)&intsize);
#elif __APPLE__
		getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, (int *)&intsize);
#else
                getsockopt(fd, SOL_SOCKET, SO_ERROR, &n, (unsigned int *)&intsize);
#endif
                if (n) {
                    if (log_flags.net_xfer_debug) {
                        printf("socket %d connect failed\n", fd);
                    }
                    nxp->error = ERR_CONNECT;
                    nxp->io_done = true;
                } else {
                    if (log_flags.net_xfer_debug) {
                        printf("socket %d is connected\n", fd);
                    }
                    nxp->is_connected = true;
                    bytes_transferred += 1;
                }
            } else if (nxp->do_file_io) {
                if (max_bytes > 0) {
                    retval = nxp->do_xfer(n);
                    max_bytes -= n;
                    bytes_transferred += n;
                }
            } else {
                nxp->io_ready = true;
            }
	} else if (FD_ISSET(fd, &error_fds)) {
            if (log_flags.net_xfer_debug) printf("got error on socket %d\n", fd);
            nxp = lookup_fd(fd);
	    nxp->got_error();
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
    int n, m, nleft, offset;
    char* buf = (char*)malloc(blocksize);

    nbytes_transferred = 0;

    if (!buf) return ERR_MALLOC;
    if (want_download) {
#ifdef _WIN32
    n = recv(socket, buf, blocksize, 0);
#else
    n = read(socket, buf, blocksize);
#endif
    if (log_flags.net_xfer_debug) {
        printf("read %d bytes from socket %d\n", n, socket);
    }
    if (n == 0) {
        io_done = true;
        want_download = false;
        goto done;
    } else if (n < 0) {
        io_done = true;
        error = ERR_READ;
        goto done;
    } else {
        nbytes_transferred += n;
        m = fwrite(buf, 1, n, file);
        if (n != m) {
            io_done = true;
            error = ERR_FWRITE;
            goto done;
	}
    }
    } else if (want_upload) {
	m = fread(buf, 1, blocksize, file);
        if (m == 0) {
            want_upload = false;
            io_done = true;
            goto done;
        } else if (m < 0) {
	    io_done = true;
	    error = ERR_FREAD;
	    goto done;
	}
	nleft = m;
	offset = 0;
	while (nleft) {
#ifdef _WIN32
	    n = send(socket, buf+offset, nleft, 0);
#else
	    n = write(socket, buf+offset, nleft);
#endif
            if (log_flags.net_xfer_debug) {
                printf("wrote %d bytes to socket %d\n", n, socket);
            }
	    if (n < 0) {
                error = ERR_WRITE;
                io_done = true;
                goto done;
            } else if (n < nleft) {
                fseek( file, n+nbytes_transferred-blocksize, SEEK_CUR );
                nbytes_transferred += n;
                goto done;
            }
	    nleft -= n;
	    offset += n;
            nbytes_transferred += n;
	}
    }
done:
    free(buf);
    return 0;
}

void NET_XFER::got_error() {
    error = ERR_IO;
    io_done = true;
}
