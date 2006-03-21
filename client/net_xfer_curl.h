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
#include <curl/curl.h>


#include "network.h"

#ifndef _MAX_PATH
#define _MAX_PATH 256
#endif 

extern CURLM* g_curlMulti; // the global libcurl multi handle 

// The following classes implement polling (non-blocking) I/O
// between a disk file (or memory block) and a socket

// global functions for starting & stopping libcurl
extern int curl_init();
extern int curl_cleanup();

// represents a network connection, either being accessed directly
// or being transferred to/from a file
//

/*
net_xfer object status codes (using libcurl):

nxf->response          maps to HTTP_STATUS_* (from http_curl.h)
nxf->http_op_state     maps to HTTP_STATE_*  (from http_curl.h)
nxf->CurlError is a curl specific code  (maps to the CURLE_* enums in curl/curl.h)
*/

class NET_XFER {
public:
    FILE* fileIn;
	FILE* fileOut;  // CMC need an output file for POST responses
	CURL* curlEasy; // the "easy curl" handle for this net_xfer request
	struct curl_slist *pcurlList; // curl slist for http headers
	struct curl_httppost *pcurlFormStart; // a pointer to a form item for POST
	struct curl_httppost *pcurlFormEnd; // a pointer to a form item for POST
	unsigned char* pByte;  // pointer to bytes for reading via libcurl_read function
	long lSeek;  // this is a pointer within the file we're reading
    char infile[_MAX_PATH];
    char outfile[_MAX_PATH];
	bool bTempOutfile; // CMC -- flag that outfile is really a tempfile we should delete
    char* req1;
	bool bSentHeader;  // CMC -- a flag that I already sent the header
	CURLcode CurlResult;   // CMC -- send up curl result code

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
    bool io_ready;
        // Signals higher layers that they can read or write socket now
        // (used if !do_file_io)
    long error;
	long response;
    double start_time;
    double xfer_speed;
    double bytes_xferred;   // bytes transferred in this session
	double content_length;

	// CMC - moved from http_op
	int http_op_state;     // values below
    int http_op_type;
    int http_op_retval;

    // save authorization types supported by proxy/socks server
    bool auth_flag;       // TRUE = server uses authorization
    long auth_type;       // 0 = haven't contacted server yet.

    NET_XFER();
    ~NET_XFER();
    void reset();
    void init();
    int get_ip_addr(int &ip_addr);
    void close_socket();
    void close_file();
    void update_speed();
    void got_error();
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
    int last_time;
    int insert(NET_XFER*);
    int remove(NET_XFER*);

    void get_fdset(FDSET_GROUP&);
    void got_select(FDSET_GROUP&, double);
    NET_XFER* lookup_curl(CURL* pcurl);   // lookup by easycurl handle
};

#endif  // _H
