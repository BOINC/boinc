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


// The following classes implement polling (non-blocking) I/O
// between a disk file (or memory block) and a socket

#define MAX_BLOCKSIZE   16384


// represents a network connection, either being accessed directly
// or being transferred to/from a file
//
class NET_XFER {
public:
    char hostname[256];     // The host we're connecting to (possibly a proxy)
    int socket;
    bool is_connected;
    bool want_download;     // at most one should be true
    bool want_upload;
    bool do_file_io;
        // If true: poll() should transfer data to/from file
        // (in which case "file" and blocksize are relevant)
        // If false: set io_ready (higher layers will do I/O)
    bool io_done;
        // set to true when the current transfer is over:
        // - the transfer timed out (not activity for a long time)
        // - network connect failed
        // - got EOF on socket read (0 bytes, select indicated I/O ready)
        // - error on disk write (e.g. volume full)
        // - reached end of disk file on upload
        // - got file read error on upload
        // - write to socket failed on upload
    FILE* file;
    bool io_ready;
        // Signals higher layers that they can read or write socket now
        // (used if !do_file_io)
    int error;
    int port;
    int blocksize;
    double start_time;
    double xfer_speed;
    double bytes_xferred;   // bytes transferred in this session
    char file_read_buf[MAX_BLOCKSIZE];
    int file_read_buf_offset, file_read_buf_len;
    int seconds_until_timeout;

    void init(char* host, int port, int blocksize);
    int get_ip_addr(int &ip_addr);
    int open_server();
    void close_socket();
    int do_xfer(int&);
    void update_speed();
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
    std::vector<NET_XFER*> net_xfers;
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
    int last_time;
    int insert(NET_XFER*);
    int remove(NET_XFER*);
    bool poll();
    int net_sleep(double);
    int do_select(double& bytes_transferred, double timeout);
    NET_XFER* lookup_fd(int);   // lookup by fd
    void check_active(bool&, bool&);
};

#endif
