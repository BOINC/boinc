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

#ifndef _NET_XFER_
#define _NET_XFER_

#ifndef _WIN32
#include <stdio.h>
#include <time.h>
#include <vector>
#endif


// The following classes implement polling (non-blocking) I/O
// between a disk file (or memory block) and a socket

#define MAX_BLOCKSIZE   16384

// represents a network connection, either being accessed directly
// or being transferred to/from a file
//
class NET_XFER {
    char hostname[256];
        // The host we're connecting to (possibly a proxy)
public:
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

    void init(char* host, int port, int blocksize);
    int get_ip_addr(int &ip_addr);
    int get_ip_addr(char *hostname, int &ip_addr);
    int open_server();
    void close_socket();
    int do_xfer(int&);
    void update_speed(int);
    void got_error();
    char* get_hostname();
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
