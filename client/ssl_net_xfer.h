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

#ifndef _NET_XFER_
#define _NET_XFER_

#ifndef _WIN32
#include <stdio.h>
#include <time.h>
#include <vector>
#endif


#ifdef _SSL_USE_
namespace INCSSL{
    #include <openssl/ssl.h>
}
using namespace INCSSL;
#endif

// The following classes implement polling (non-blocking) I/O
// between a disk file (or memory block) and a socket

#define MAX_BLOCKSIZE   16384

#ifdef _SSL_USE_
#define SSL_READ_MODE    1
#define SSL_WRITE_MODE   2
#endif

// represents a network connection, either being accessed directly
// or being transferred to/from a file
//
class NET_XFER {
    char hostname[256];
        // The host we're connecting to (possibly a proxy)
public:
#ifdef _SSL_USE_
    NET_XFER();
    ~NET_XFER();
#endif
    int socket;
    bool is_connected;
    bool want_download;     // at most one should be true
    bool want_upload;
    bool do_file_io;
        // whether poll() should transfer data to/from file
        // (in which case "file" and blocksize are relevant)
        // or just set io_ready
    bool io_done;           // got error or EOF
    FILE* file;
    bool io_ready;
        // can read or write socket now (used if !do_file_io)
    int error;
    int port;
    int blocksize;
    double start_time;
    double xfer_speed;
    double bytes_xferred;   // bytes transferred in this session
    char file_read_buf[MAX_BLOCKSIZE];
    int file_read_buf_offset, file_read_buf_len;
#ifdef _SSL_USE_
    INCSSL::SSL *ssl; 
    INCSSL::SSL_CTX *ctx;
    int url_mode;
    bool proxy_connect_finished;
#endif
    int seconds_until_timeout;

    void init(char* host, int port, int blocksize);
#ifdef _SSL_USE_
    void init_2();
    void init_ssl();
    int close_ssl();
    int ssl_connect(int, int*);
    int ssl_read_write(int, char*, int, int, int*);
    void ssl_error_print(char*, int, int);
#endif
    int get_ip_addr(int &ip_addr);
    int get_ip_addr(char *hostname, int &ip_addr);
    int open_server();
    void close_socket();
    int do_xfer(int&);
    void update_speed(int);
    void got_error();
    char* get_hostname();
    bool check_timeout(bool);
    void reset_timeout();
};

// bandwidth limitation is implemented at this level, as follows:
// There are limits max_bytes_sec_up and max_bytes_sec_down.
// We keep track of the last time and bytes_left_up and bytes_left_down;
// Each second we reset these to zero.

class NET_XFER_SET {
    vector<NET_XFER*> net_xfers;
public:
    NET_XFER_SET();
    double max_bytes_sec_up, max_bytes_sec_down;
        // user-specified limits on throughput
    double bytes_left_up, bytes_left_down;
        // bytes left to transfer in the current second
    double bytes_up, bytes_down;
        // total bytes transferred
    bool up_active, down_active;
        // has there been transfer activity since last call to check_active()?
    time_t last_time;
    int insert(NET_XFER*);
    int remove(NET_XFER*);
    bool poll();
    int net_sleep(double);
    int do_select(double& bytes_transferred, double timeout);
    NET_XFER* lookup_fd(int);   // lookup by fd
    void check_active(bool&, bool&);
};

extern int get_socket_error(int fd);

#endif
