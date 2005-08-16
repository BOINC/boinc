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
extern int curl_init()
{
	curl_global_init(CURL_GLOBAL_ALL);
	g_curlMulti = curl_multi_init();
	return (int)(g_curlMulti == NULL);
}

extern int curl_cleanup()
{
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
	if (curlEasy && pcurlFormStart)
	{
		curl_formfree(pcurlFormStart);
		curl_formfree(pcurlFormEnd);
		pcurlFormStart = pcurlFormEnd = NULL;
	}
	if (curlEasy && g_curlMulti)
	{  // release this handle
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

// Transfer data to/from active sockets.
// Keep doing I/O until would block, or we hit rate limits,
// or .5 second goes by
//
bool NET_XFER_SET::poll() {
    double bytes_xferred =0.0f;
    int retval;
    bool action = false;
	// CMC probably don't need this loop in libcurl, the regular polling interval should suffice?
//    while (1) {
        retval = do_select(bytes_xferred, 0.5);
        if (retval) return false;
        if (bytes_xferred ==0) return false;
        return true;
//        if (retval) break;
//        if (bytes_xferred == 0) break;
//        action = true;
//        if ((dtime() - gstate.now) > 0.5) break;
//    }
	//bInHere = false;
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
	// CMC -- this seems to be the culprit for the race condition
	// as we're polling from client_state::do_something() as well
	// as from main->client_state::net_sleep()
	// need to ensure that the do_select isn't instantaneous --
	// i.e. it will boinc_sleep if no network transactions

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
    int iNumMsg;
	NET_XFER* nxf = NULL;
    //struct timeval tv;
    bool time_passed = false;
	CURLMsg *pcurlMsg = NULL;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);
	bytes_transferred = 0;

	if (!net_xfers.size()) {
		boinc_sleep(timeout); // sleep a little, don't just return
		return 1; // no pending or running net transactions
	}

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

	int max_fd = 0, iRunning = 0;  // curl flags for max # of fds & # running queries
	CURLMcode curlMErr;
	CURLcode curlErr;
    //double_to_timeval(timeout, tv);

	// get the data waiting for transfer in or out
	// note that I use timeout value so that we don't "hog" in this loop
    bool got_data = false;
    while (1) {
        curlMErr = curl_multi_perform(g_curlMulti, &iRunning);
        if (curlMErr != CURLM_CALL_MULTI_PERFORM) break;
        if (dtime() - gstate.now > timeout) break;
        got_data = true;
    }
    if (!got_data) {
        boinc_sleep(timeout);
    }

	// read messages from curl that may have come in from the above loop
	while ( (pcurlMsg = curl_multi_info_read(g_curlMulti, &iNumMsg)) )
	{
		// if we have a msg, then somebody finished
		// can check also with pcurlMsg->msg == CURLMSG_DONE
		if ((nxf = lookup_curl(pcurlMsg->easy_handle)) ) { 
			 // we have a message from one of our http_ops
			 // get the response code for this request
			curlErr = curl_easy_getinfo(nxf->curlEasy, 
				CURLINFO_RESPONSE_CODE, &nxf->response);
			curlErr = curl_easy_getinfo(nxf->curlEasy, 
				CURLINFO_OS_ERRNO, &nxf->error);

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
			nxf->http_op_retval = nxf->response - 200;  

			if (!nxf->http_op_retval && nxf->http_op_type == HTTP_OP_POST2) {
				// for a successfully completed request on a "post2" --
				// read in the temp file into req1 memory
				fclose(nxf->fileOut);
				double dSize = 0.0f;
				file_size(nxf->outfile, dSize);
				nxf->fileOut = boinc_fopen(nxf->outfile, "rb");
				if (!nxf->fileOut)
				{ // ack, can't open back up!
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
			if (nxf->bTempOutfile)
				boinc_delete_file(nxf->outfile);
		}
	}

	// reset and get curl fds
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);

	// "prime" the fdset for the next libcurl multi op 
	curlMErr = curl_multi_fdset(g_curlMulti, &read_fds, &write_fds, &error_fds, &max_fd);

	/* CMC - this is dumb, if data coming in, don't sleep on it!
	// need to sleep any leftover time
	double dSleepItOff = timeout - (double)(time(0) - t);
	if (dSleepItOff > 0.0f)
		boinc_sleep(dSleepItOff);
		*/

	return 0;

	/* CMC obsolete
	if (curlMErr != CURLM_OK) {
		return ERR_SELECT; // something wrong
	}
	else if (max_fd == -1) { // no fd's, nothing to do!
		up_active = down_active = false;
		return retval;
	}
    // do a select on all libcurl handles
	// http://curl.haxx.se/libcurl/c/curl_multi_fdset.html
	timeval tv_curl;
	tv_curl.tv_sec = 5;
	tv_curl.tv_usec = 0;  // libcurl docs say use a small (single digit) # of seconds
	n = select(max_fd+1, &read_fds, &write_fds, &error_fds, &tv_curl);
	
	switch (n) {
		case -1:  // error on select
			return ERR_SELECT; 
			break;
		case 0: 
			return 1; 
			break;
		default:
			break;
	}

	// at this point, libcurl should have done all the pending transfers
	// now go interate net_xfers -- examine each one for status, errors, etc
	return 1;
	*/
}

// Return the NET_XFER object whose socket matches fd
NET_XFER* NET_XFER_SET::lookup_curl(CURL* pcurl) 
{
    for (unsigned int i=0; i<net_xfers.size(); i++) {
        if (net_xfers[i]->curlEasy == pcurl) {
            return net_xfers[i];
        }
    }
    return 0;
}

/*  CMC not needed for libcurl

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
*/

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