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
#include "config.h"
#include <cstdio>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <arpa/inet.h>
#include <sys/types.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
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

CURLM* g_curlMulti = NULL;

// the file descriptor sets need to be global so libcurl has access always
//
fd_set read_fds, write_fds, error_fds;

// call these once at the start of the program and once at the end
//
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
    CurlResult = CURLE_OK;
    bTempOutfile = true;
    is_connected = false;
    want_download = false;
    want_upload = false;
    do_file_io = true;
    io_done = false;
    fileIn = NULL;
    fileOut = NULL;
    io_ready = true;
    error = 0;
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
    auth_flag = false;
    auth_type = 0;
    reset();
}

NET_XFER::~NET_XFER() {
    close_socket();
    close_file();
}

void NET_XFER::close_socket() {
    // this cleans up the curlEasy, and "spoofs" the old close_socket
    //
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

void NET_XFER::init() {
    reset();
    start_time = gstate.now;
}

NET_XFER_SET::NET_XFER_SET() {
    max_bytes_sec_up = 0;
    max_bytes_sec_down = 0;
    bytes_left_up = 0;
    bytes_left_down = 0;
    bytes_up = 0;
    bytes_down = 0;
}

int NET_XFER_SET::insert(NET_XFER* nxp) {
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
    msg_printf(NULL, MSG_ERROR, "Network transfer object not found");
    return ERR_NOT_FOUND;
}


void NET_XFER_SET::get_fdset(FDSET_GROUP& fg) {
    CURLMcode curlMErr;
    curlMErr = curl_multi_fdset(
        g_curlMulti, &fg.read_fds, &fg.write_fds, &fg.exc_fds, &fg.max_fd
    );
    //printf("curl msfd %d %d\n", curlMErr, fg.max_fd);
}

void NET_XFER_SET::got_select(FDSET_GROUP&, double timeout) {
    int iNumMsg;
    NET_XFER* nxf = NULL;
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
    // use timeout value so that we don't hog CPU in this loop
    //
    while (1) {
        curlMErr = curl_multi_perform(g_curlMulti, &iRunning);
        if (curlMErr != CURLM_CALL_MULTI_PERFORM) break;
        gstate.need_physical_connection = false;
        if (dtime() - gstate.now > timeout) break;
    }

    // read messages from curl that may have come in from the above loop
    //
    while ((pcurlMsg = curl_multi_info_read(g_curlMulti, &iNumMsg))) {
        // if we have a msg, then somebody finished
        // can check also with pcurlMsg->msg == CURLMSG_DONE
        //
        nxf = lookup_curl(pcurlMsg->easy_handle);
        if (!nxf) continue;

         // we have a message from one of our http_ops
         // get the response code for this request
        //
        curlErr = curl_easy_getinfo(nxf->curlEasy, 
            CURLINFO_RESPONSE_CODE, &nxf->response
        );

        // CURLINFO_LONG+25 is a workaround for a bug in the gcc version
        // included with Mac OS X 10.3.9
        //
        curlErr = curl_easy_getinfo(nxf->curlEasy, 
            (CURLINFO)(CURLINFO_LONG+25) /*CURLINFO_OS_ERRNO*/, &nxf->error
        );

        nxf->io_done = true;
        nxf->io_ready = false;

        // update byte counts and transfer speed
        //
        if (nxf->want_download) {
            bytes_down += nxf->bytes_xferred;
            curlErr = curl_easy_getinfo(nxf->curlEasy, 
                CURLINFO_SPEED_DOWNLOAD, &nxf->xfer_speed
            );
        }
        if (nxf->want_upload) {
            bytes_up += nxf->bytes_xferred;  
            curlErr = curl_easy_getinfo(nxf->curlEasy, 
                CURLINFO_SPEED_UPLOAD, &nxf->xfer_speed
            );
        }

        // if proxy/socks server uses authentication and its not set yet,
        // get what last transfer used
        if (nxf->auth_flag && !nxf->auth_type) {
            curlErr = curl_easy_getinfo(nxf->curlEasy, 
                CURLINFO_PROXYAUTH_AVAIL, &nxf->auth_type);
        }

        // the op is done if curl_multi_msg_read gave us a msg for this http_op
        //
        nxf->http_op_state = HTTP_STATE_DONE;        
        nxf->CurlResult = pcurlMsg->data.result;

        if (nxf->CurlResult == CURLE_OK) {
            if ((nxf->response/100)*100 == HTTP_STATUS_OK) {
                nxf->http_op_retval = 0;  
            } else if ((nxf->response/100)*100 == HTTP_STATUS_CONTINUE) {
                continue;
            } else {
                nxf->http_op_retval = nxf->response;
            }
            gstate.need_physical_connection = false;
        } else {
            // If operation failed,
            // it could be because there's no physical network connection.
            // Find out for sure by trying to contact google
            //
            if (!gstate.lookup_website_op.checking_network) {
                gstate.lookup_website_op.checking_network = true;
                std::string url = "http://www.google.com";
                gstate.lookup_website_op.do_rpc(url);
            }
            msg_printf(0, MSG_ERROR,
                "HTTP error: %s", curl_easy_strerror(nxf->CurlResult)
            );
            nxf->http_op_retval = ERR_HTTP_ERROR;
        }

        if (!nxf->http_op_retval && nxf->http_op_type == HTTP_OP_POST2) {
            // for a successfully completed request on a "post2" --
            // read in the temp file into req1 memory
            //
            fclose(nxf->fileOut);
            double dSize = 0.0f;
            file_size(nxf->outfile, dSize);
            nxf->fileOut = boinc_fopen(nxf->outfile, "rb");
            if (!nxf->fileOut) { // ack, can't open back up!
                nxf->response = 1;
                    // flag as a bad response for a possible retry later
            } else {
                fseek(nxf->fileOut, 0, SEEK_SET);
                // CMC Note: req1 is a pointer to "header" which is 4096
                memset(nxf->req1, 0, 4096);
                fread(nxf->req1, 1, (size_t) dSize, nxf->fileOut); 
            }
        }

        // close files and "sockets" (i.e. libcurl handles)
        //
        nxf->close_file();
        nxf->close_socket();

        // finally remove the tmpfile if not explicitly set
        //
        if (nxf->bTempOutfile) {
            boinc_delete_file(nxf->outfile);
        }
    }
}

// Return the NET_XFER object with given Curl object
//
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


const char *BOINC_RCSID_e0a7088e04 = "$Id$";
