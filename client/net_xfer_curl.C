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

#ifdef _USE_CURL

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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#endif

#include "error_numbers.h"
#include "net_xfer_curl.h"
#include "util.h"
#include "network.h"
#include "filesys.h"

#include "client_types.h"
#include "client_state.h"
#include "client_msgs.h"

using std::vector;

// if an active transfer doesn't get any activity
// in this many seconds, error out
//
#define NET_XFER_TIMEOUT    600

CURLM* g_curlMulti = NULL;  // global curl for this module, can handle http & https

// the file descriptor sets need to be global so libcurl has access always
fd_set read_fds, write_fds, error_fds;

// call these once at the start of the program and once at the end (init & cleanup of course)
int curl_init() {
	curl_global_init(CURL_GLOBAL_ALL);
	g_curlMulti = curl_multi_init();
	return (int)(g_curlMulti == NULL);
}

int curl_cleanup() {
	if (g_curlMulti) {
		curl_multi_cleanup(g_curlMulti);
	}
	return 0;
}

void NET_XFER::reset() {
    req1 = NULL;
    strcpy(infile, "");
    strcpy(outfile, "");
	bTempOutfile = true;
    is_connected = false;
    want_download = false;
    want_upload = false;
    do_file_io = true;  // CMC Note: should I default to true, i.e. this does all i/o?
    io_done = false;
    fileIn = NULL;
	fileOut = NULL;
    io_ready = true;  // don't allow higher levels to do i/o?
    error = 0;
    file_read_buf_offset = 0;
    file_read_buf_len = 0;
    bytes_xferred = 0;
    xfer_speed = 0;
	bSentHeader = false;
	close_socket();
}

NET_XFER::NET_XFER() {
	pcurlList = NULL; // these have to be NULL, just in constructor
	curlEasy = NULL;
	pcurlFormStart = NULL;
	pcurlFormEnd = NULL;
	pByte = NULL;
	lSeek = 0;
    reset();
}

NET_XFER::~NET_XFER() {
    close_socket();
    close_file();
}

void NET_XFER::close_socket() {
	// CMC: this just cleans up the curlEasy, and "spoofs" the old close_socket
	if (pcurlList) {
		curl_slist_free_all(pcurlList);
		pcurlList = NULL;
	}
	if (curlEasy && pcurlFormStart) {
		curl_formfree(pcurlFormStart);
		curl_formfree(pcurlFormEnd);
		pcurlFormStart = pcurlFormEnd = NULL;
	}
	if (curlEasy && g_curlMulti) {  // release this handle
		curl_multi_remove_handle(g_curlMulti, curlEasy);
		curl_easy_cleanup(curlEasy);
		curlEasy = NULL;
	}
}

void NET_XFER::close_file() {
    if (fileIn) {
        fclose(fileIn);
        fileIn = NULL;
    }
    if (fileOut) {
        fclose(fileOut);
        fileOut = NULL;
    }
	if (pByte) { //free any read memory used
		delete [] pByte;
		pByte = NULL;
	}
}

void NET_XFER::init(char* host, int p, int b) {
    reset();
    safe_strcpy(hostname, host);
    port = p;
    blocksize = (b > MAX_BLOCKSIZE ? MAX_BLOCKSIZE : b);
    start_time = gstate.now;
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

// Connect to a server,
// and if successful insert the NET_XFER object into the set
//
int NET_XFER_SET::insert(NET_XFER* nxp) {
    //int retval = nxp->open_server();
    //if (retval) return retval;
    net_xfers.push_back(nxp);
    return 0;
}

// Remove a NET_XFER object from the set
//
int NET_XFER_SET::remove(NET_XFER* nxp) {
    vector<NET_XFER*>::iterator iter;

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


void NET_XFER_SET::get_fdset(FDSET_GROUP& fg) {
	CURLMcode curlMErr;
	curlMErr = curl_multi_fdset(g_curlMulti, &fg.read_fds, &fg.write_fds, &fg.exc_fds, &fg.max_fd);
    //printf("curl msfd %d %d\n", curlMErr, fg.max_fd);
}

void NET_XFER_SET::got_select(FDSET_GROUP&, double timeout) {
    int iNumMsg;
	NET_XFER* nxf = NULL;
    //struct timeval tv;
    bool time_passed = false;
	CURLMsg *pcurlMsg = NULL;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    // if a second has gone by, do rate-limit accounting
    //
    time_t t = time(0);
    if (t != last_time) {
        time_passed = true;
        last_time = (int)t;
        if (bytes_left_up < max_bytes_sec_up) {
            bytes_left_up += max_bytes_sec_up;
        }
        if (bytes_left_down < max_bytes_sec_down) {
            bytes_left_down += max_bytes_sec_down;
        }
    }

    int iRunning = 0;  // curl flags for max # of fds & # running queries
	CURLMcode curlMErr;
	CURLcode curlErr;

	// get the data waiting for transfer in or out
	// note that I use timeout value so that we don't "hog" in this loop
    bool got_data = false;
    while (1) {
        curlMErr = curl_multi_perform(g_curlMulti, &iRunning);
        if (curlMErr != CURLM_CALL_MULTI_PERFORM) break;
        if (dtime() - gstate.now > timeout) break;
        got_data = true;
    }

	// read messages from curl that may have come in from the above loop
	while ((pcurlMsg = curl_multi_info_read(g_curlMulti, &iNumMsg))) {
		// if we have a msg, then somebody finished
		// can check also with pcurlMsg->msg == CURLMSG_DONE
		if ((nxf = lookup_curl(pcurlMsg->easy_handle)) ) { 
                        // CURLINFO ci=CURLINFO_OS_ERRNO;
			 // we have a message from one of our http_ops
			 // get the response code for this request
			curlErr = curl_easy_getinfo(nxf->curlEasy, 
				CURLINFO_RESPONSE_CODE, &nxf->response);
                        // CURLINFO_LONG+25 is a workaround for a bug in the gcc version included
                        // with Mac OS X 10.3.9
			curlErr = curl_easy_getinfo(nxf->curlEasy, 
				(CURLINFO)(CURLINFO_LONG+25) /*CURLINFO_OS_ERRNO*/, &nxf->error);

			nxf->io_done = true;
			nxf->io_ready = false;

			if (nxf->want_download) {
				bytes_down += nxf->bytes_xferred;
				// get xfer speed (don't mess with bytes_xferred, that's in write function
				curlErr = curl_easy_getinfo(nxf->curlEasy, 
					CURLINFO_SPEED_DOWNLOAD, &nxf->xfer_speed);
			}
			if (nxf->want_upload) {
				bytes_up += nxf->bytes_xferred;
				// get xfer speed (don't mess with bytes_xferred, that's in write function
				curlErr = curl_easy_getinfo(nxf->curlEasy, 
					CURLINFO_SPEED_UPLOAD, &nxf->xfer_speed);
			}

			// the op is done if curl_multi_msg_read gave us a msg for this http_op
			nxf->http_op_state = HTTP_STATE_DONE;

			// 200 is a good HTTP response code
			// It may not mean the data received is "good"
			// (the calling program will have to check/parse that)
			// but it at least means that the server operation
			// went through fine
            if ((nxf->response/100)*100 != HTTP_STATUS_OK) {
                nxf->http_op_retval = nxf->response;
            } else {
			    nxf->http_op_retval = nxf->response - 200;  
            }

			if (!nxf->http_op_retval && nxf->http_op_type == HTTP_OP_POST2) {
				// for a successfully completed request on a "post2" --
				// read in the temp file into req1 memory
				fclose(nxf->fileOut);
				double dSize = 0.0f;
				file_size(nxf->outfile, dSize);
				nxf->fileOut = boinc_fopen(nxf->outfile, "rb");
				if (!nxf->fileOut) { // ack, can't open back up!
					nxf->response = 1; // flag as a bad response for a possible retry later
				}
				fseek(nxf->fileOut, 0, SEEK_SET);
				// CMC Note: req1 is a pointer to "header" which is 4096
				memset(nxf->req1, 0x00, 4096);
				fread(nxf->req1, 1, (size_t) dSize, nxf->fileOut); 
			}

			// close files and "sockets" (i.e. libcurl handles)
			nxf->close_file();
			nxf->close_socket();

			// finally remove the tmpfile if not explicitly set
            if (nxf->bTempOutfile) {
				boinc_delete_file(nxf->outfile);
            }
		}
	}
}

// Return the NET_XFER object whose socket matches fd
NET_XFER* NET_XFER_SET::lookup_curl(CURL* pcurl)  {
    for (unsigned int i=0; i<net_xfers.size(); i++) {
        if (net_xfers[i]->curlEasy == pcurl) {
            return net_xfers[i];
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
#if 0
    // TODO: figure out what to do here
    } else if (xfer_speed == 0) {
        xfer_speed = 999999999;
#endif
    }
}

void NET_XFER::got_error() {
    // TODO: which socket??
    error = ERR_IO;
    io_done = true;
    log_messages.printf(
        CLIENT_MSG_LOG::DEBUG_NET_XFER, "IO error on socket\n"
    );
}

// return true if an upload is currently in progress
// or has been since the last call to this.
// Similar for download.
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

#endif //_USE_CURL
