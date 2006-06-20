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
#else
#include "config.h"
#include <cstring>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>
#endif

#include "error_numbers.h"
#include "filesys.h"
#include "client_msgs.h"
#include "log_flags.h"
#include "util.h"

//#include "network.h"
#include "client_msgs.h"
#include "base64.h"
#include "http_curl.h"

#define CONNECTED_STATE_NOT_CONNECTED   0
#define CONNECTED_STATE_CONNECTED       1
#define CONNECTED_STATE_UNKNOWN         2

#define HTTP_BLOCKSIZE  16384

using std::min;
using std::vector;

extern CURLM* g_curlMulti;  // global curl multi handle for http/s
static char g_user_agent_string[256] = {""};
//static const char g_content_type[] = {"Content-Type: application/octet-stream"};
// CMC Note: old BOINC used the above, but libcurl seems to like the following:
static const char g_content_type[] = {"Content-Type: application/x-www-form-urlencoded"};


/** as an example of usage of http_op -- the scheduler typically requires 
    just the following:

http_op

http_op_state flag
http_op_retval 

set_proxy     (g_state.proxy_info)
init_get()
init_post()

http_ops
   -> insert
      remove

*/

// Breaks a HTTP URL down into its server, port and file components
// format of url:
// [http[s]://]host.dom.dom[:port][/dir/file]
//
void parse_url(const char* url, char* host, int &port, char* file) {
    char* p;
    char buf[256];
    bool bSSL = false;

    //const short csiLen = bSSL ? 8 : 7; // get right size of http/s string comparator

    // strip off http:// if present
    //
    //if (strncmp(url, (bSSL ? "https://" : "http://"), csiLen) == 0) {
    if (strncmp(url, "http://", 7) == 0) {
        safe_strcpy(buf, url+7);
    } else { 
        // wait, may be https://
        if (strncmp(url, "https://", 8) == 0) {
            safe_strcpy(buf, url+8);
            bSSL = true; // retain the fact that this was a secure http url
        }
        else { // no http:// or https:// prepended on url
            safe_strcpy(buf, url);
        }
    }

    // parse and strip off file part if present
    //
    p = strchr(buf, '/');
    if (p) {
        strcpy(file, p+1);
        *p = 0;
    } else {
        strcpy(file, "");
    }

    // parse and strip off port if present
    //
    p = strchr(buf,':');
    if (p) {
        port = atol(p+1);
        *p = 0;
    } else {
        // CMC note:  if they didn't pass in a port #, 
        //    but the url starts with https://, assume they
        //    want a secure port (HTTPS, port 443)
        port = (bSSL ? 443 : 80);  
    }

    // what remains is the host
    //
    strcpy(host, buf);
}

void get_user_agent_string() {
    sprintf(g_user_agent_string, "BOINC client (%s %d.%d.%d)",
        HOSTTYPE, BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE
    );
}

HTTP_OP::HTTP_OP() {
    strcpy(m_url, "");
    content_length = 0;
    file_offset = 0;
    strcpy(request_header, "");
    http_op_state = HTTP_STATE_IDLE;
    http_op_type = HTTP_OP_NONE;
    http_op_retval = 0;
    trace_id = 0;
}

HTTP_OP::~HTTP_OP() {
}

// Initialize HTTP GET operation
//
int HTTP_OP::init_get(
    const char* url, const char* out, bool del_old_file, double off
) {
    //char proxy_buf[256];

    if (del_old_file) {
        unlink(out);
    }
    req1 = NULL;  // not using req1, but init_post2 uses it
    file_offset = off;
    NET_XFER::init();
    // usually have an outfile on a get
    if (off != 0) {
        bytes_xferred = off;
    }
    http_op_type = HTTP_OP_GET;
    http_op_state = HTTP_STATE_CONNECTING;
    return HTTP_OP::libcurl_exec(url, NULL, out, off, false);
}

// Initialize HTTP POST operation
//
int HTTP_OP::init_post(
    const char* url, const char* in, const char* out
) {
    int retval;
    double size;

    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_HTTP);
    req1 = NULL;  // not using req1, but init_post2 uses it

    if (in) {
        // we should pretty much always have an in file for _post, optional in _post2
        strcpy(infile, in);
        retval = file_size(infile, size);
        if (retval) return retval;  // this will return 0 or ERR_NOT_FOUND
        content_length = (int)size;
    }
    NET_XFER::init();
    http_op_type = HTTP_OP_POST;
    http_op_state = HTTP_STATE_CONNECTING;
    scope_messages.printf("HTTP_OP::init_post(): %p io_done %d\n", this, io_done);
    return HTTP_OP::libcurl_exec(url, in, out, 0.0, true);  // note that no offset for this, for resumable uploads use post2!
}

// the following will do an HTTP GET or POST using libcurl,
// polling at the net_xfer level
//
int HTTP_OP::libcurl_exec(
    const char* url, const char* in, const char* out, double offset, bool bPost
) {
    CURLMcode curlMErr;
    CURLcode curlErr;
    char strTmp[128];

    safe_strcpy(m_url, url);

    if (g_user_agent_string[0] == 0x00) {
        get_user_agent_string();
    }

    if (in) {
        strcpy(infile, in);
    }
    if (out) {
        bTempOutfile = false;
        strcpy(outfile, out);
    } else {
        //CMC -- I always want an outfile for the server response, delete when op done
        bTempOutfile = true;
        memset(outfile, 0x00, _MAX_PATH);
#if defined(_WIN32) && !defined(__CYGWIN32__)
        char* ptrName;
        ptrName = _tempnam("./", "blc");
        if (ptrName) {
            strcpy(outfile, ptrName);
            free(ptrName);
        }
#elif defined( __EMX__)
        strcpy(outfile, "blcXXXXXX"); // a template for the mktemp
       // mktemp will not open the file
        mktemp(outfile);
#else  // use mkstemp on Mac & Linux due to security issues
        strcpy(outfile, "blcXXXXXX"); // a template for the mkstemp
        close(mkstemp(outfile));
#endif
    }

    // setup libcurl handle
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    // CMC -- init the curlEasy handle and setup options
    //   the polling will do the actual start of the HTTP/S transaction

    curlEasy = curl_easy_init(); // get a curl_easy handle to use
    if (!curlEasy) {
        msg_printf(0, MSG_ERROR, "Couldn't create curlEasy handle");
        return ERR_HTTP_ERROR; // returns 0 (CURLM_OK) on successful handle creation
    }

    // OK, we have a handle, now open an asynchronous libcurl connection

    // set the URL to use
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_URL, m_url);

    /*
    CURLOPT_SSL_VERIFYHOST

Pass a long as parameter.

This option determines whether curl verifies that the server claims to be who you want it to be.

When negotiating an SSL connection, the server sends a certificate indicating its identity.

When CURLOPT_SSL_VERIFYHOST is 2, that certificate must indicate that the server is the server to which you meant to connect, or the connection fails.

Curl considers the server the intended one when the Common Name field or a Subject Alternate Name field in the certificate matches the host name in the URL to which you told Curl to connect.

When the value is 1, the certificate must contain a Common Name field, but it doesn't matter what name it says. (This is not ordinarily a useful setting).

When the value is 0, the connection succeeds regardless of the names in the certificate.

The default, since 7.10, is 2.

The checking this option controls is of the identity that the server claims. The server could be lying. To control lying, see CURLOPT_SSL_VERIFYPEER. 
    */
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYHOST, 2L);

    // the following sets "tough" certificate checking
    // (i.e. whether self-signed is OK)
    // if zero below, will accept self-signed certificates
    // (cert not 3rd party trusted)
    // if non-zero below, you need a valid 3rd party CA (i.e. Verisign, Thawte)
    //
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYPEER, 1L);

    // set the user agent as this boinc client & version
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_USERAGENT, g_user_agent_string);

    // bypass any signal handlers that curl may want to install
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_NOSIGNAL, 1L);
    // bypass progress meter
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_NOPROGRESS, 1L);

    // setup timeouts
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_TIMEOUT, 0L);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_LOW_SPEED_LIMIT, 10L);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_LOW_SPEED_TIME, 300L);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_CONNECTTIMEOUT, 120L);
    
    // force curl to use HTTP/1.1
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_MAXREDIRS, 50L);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_AUTOREFERER, 1L);
    curlErr = curl_easy_setopt(curlEasy, CURLOPT_FOLLOWLOCATION, 1L);

    // if we tell Curl to accept any encoding (e.g. deflate)
    // it seems to accept them all, which screws up projects that
    // use gzip at the application level.
    // So, detect this and don't accept any encoding in that case
    //
    if (!out || !ends_with(std::string(out), std::string(".gz"))) {
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_ENCODING, "");
    }

    // setup any proxy they may need
    setupProxyCurl();

    // set the content type in the header (defined at the top -- app/octect-stream)
    pcurlList = curl_slist_append(pcurlList, g_content_type);

	// set the file offset for resumable downloads
	if (!bPost && offset>0.0f) {
        file_offset = offset;
        sprintf(strTmp, "Range: bytes=%.0f-", offset);
        pcurlList = curl_slist_append(pcurlList, strTmp);
    }

    // we'll need to setup an output file for the reply
    if (outfile && strlen(outfile)>0) {  
		// if offset>0 don't truncate!
		char szType[3] = "wb";
		if (file_offset>0.0) szType[0] = 'a';
        fileOut = boinc_fopen(outfile, szType);
        if (!fileOut) {
            msg_printf(NULL, MSG_ERROR, 
                "Can't create HTTP response output file %s", outfile
            );
            io_done = true;
            http_op_retval = ERR_FOPEN;
            http_op_state = HTTP_STATE_DONE;
            return ERR_FOPEN;
        }
        // CMC Note: we can make the libcurl_write "fancier" in the future,
        // for now it just fwrite's to the file request, which is sufficient
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_WRITEFUNCTION, libcurl_write);
        // note that in my lib_write I'm sending in a pointer to this instance of HTTP_OP
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_WRITEDATA, this);
    }

    if (bPost) {  // POST
        want_upload = true;
        want_download = false;
        if (infile && strlen(infile)>0) {
            fileIn = boinc_fopen(infile, "rb");
            if (!fileIn) {
                msg_printf(NULL, MSG_ERROR, "No HTTP input file %s", infile);
                io_done = true;
                http_op_retval = ERR_FOPEN;
                http_op_state = HTTP_STATE_DONE;
                return ERR_FOPEN;
            }
        }        

        // suppress Expect line in header (avoids problems with some proxies)
        //
        pcurlList = curl_slist_append(pcurlList, "Expect:");

        if (pcurlList) { // send custom headers if required
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, pcurlList);
        }

        // set the data file info to read for the PUT/POST
        // note the use of this curl typedef for large filesizes

        /* HTTP PUT method
        curl_off_t fs = (curl_off_t) content_length; 
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDS, NULL);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_INFILESIZE, content_length);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_READDATA, fileIn);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_INFILESIZE_LARGE, fs);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_PUT, 1L);
        */

        // HTTP POST method
        // set the multipart form for the file -- boinc just has the one section (file)
        
        /*  CMC -- if we ever want to do POST as multipart forms someday 
                   // (many seem to prefer it that way, i.e. libcurl)
                   
        pcurlFormStart = pcurlFormEnd = NULL;
        curl_formadd(&pcurlFormStart, &pcurlFormEnd, 
               CURLFORM_FILECONTENT, infile,
               CURLFORM_CONTENTSLENGTH, content_length,
               CURLFORM_CONTENTTYPE, g_content_type, 
               CURLFORM_END);
        curl_formadd(&post, &last,
               CURLFORM_COPYNAME, "logotype-image",
               CURLFORM_FILECONTENT, "curl.png", CURLFORM_END);
        */

        //curlErr = curl_easy_setopt(curlEasy, CURLOPT_HTTPPOST, pcurlFormStart);
        curl_off_t fs = (curl_off_t) content_length; 

        pByte = NULL;
        lSeek = 0;    // initialize the vars we're going to use for byte transfers

        // CMC Note: we can make the libcurl_read "fancier" in the future,
        // for now it just fwrite's to the file request, which is sufficient
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDS, NULL);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDSIZE_LARGE, fs);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_READFUNCTION, libcurl_read);
        // note that in my lib_write I'm sending in a pointer to this instance of HTTP_OP
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_READDATA, this);

        // callback function to rewind input file 
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_IOCTLFUNCTION, libcurl_ioctl);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_IOCTLDATA, this);

        curlErr = curl_easy_setopt(curlEasy, CURLOPT_POST, 1L);
    } else {  // GET
        want_upload = false;
        want_download = true;

        // now write the header, pcurlList gets freed in net_xfer_curl
        if (pcurlList) { // send custom headers if required
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, pcurlList);
        }

        // setup the GET!
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_HTTPGET, 1L);
    }

    // turn on debug info if tracing enabled
    if (log_flags.net_xfer_debug) {
        static int trace_count = 0;
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_DEBUGFUNCTION, libcurl_debugfunction);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_DEBUGDATA, this );
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_VERBOSE, 1L); 
        trace_id = trace_count++;
    }

    // last but not least, add this to the curl_multi

    curlMErr = curl_multi_add_handle(g_curlMulti, curlEasy);
    if (curlMErr != CURLM_OK && curlMErr != CURLM_CALL_MULTI_PERFORM) { // bad error, couldn't attach easy curl handle
        msg_printf(0, MSG_ERROR, "Couldn't add curlEasy handle to curlMulti");
        return ERR_HTTP_ERROR; // returns 0 (CURLM_OK) on successful handle creation
    }

    // that should about do it, the net_xfer_set polling will actually start the transaction
    // for this HTTP_OP
    return 0;
}


// Initialize HTTP POST operation
//
int HTTP_OP::init_post2(
    const char* url, char* r1, const char* in, double offset
) {
    int retval;
    double size;
    //char proxy_buf[256];

    NET_XFER::init();
    req1 = r1;
    if (in) {
        safe_strcpy(infile, in);
        file_offset = offset;
        retval = file_size(infile, size);
        if (retval) {
            printf("HTTP::init_post2: couldn't get file size\n");
            return retval; // this will be 0 or ERR_NOT_FOUND
        }
        content_length = (int)size - (int)offset;
    }
    content_length += strlen(req1);
    http_op_type = HTTP_OP_POST2;
    http_op_state = HTTP_STATE_CONNECTING;
    return HTTP_OP::libcurl_exec(url, in, NULL, offset, true);
}

// Returns true if the HTTP operation is complete
//
bool HTTP_OP::http_op_done() {
    return (http_op_state == HTTP_STATE_DONE);
}

HTTP_OP_SET::HTTP_OP_SET(NET_XFER_SET* p) {
    net_xfers = p;
}

// Adds an HTTP_OP to the set
//
int HTTP_OP_SET::insert(HTTP_OP* ho) {
    int retval;
    retval = net_xfers->insert(ho);
    if (retval) return retval;  // this will return 0 or a BOINC ERR_* message
    http_ops.push_back(ho);
    return 0;
}

// Remove an HTTP_OP from the set
//
int HTTP_OP_SET::remove(HTTP_OP* p) {
    vector<HTTP_OP*>::iterator iter;

    net_xfers->remove(p);

    iter = http_ops.begin();
    while (iter != http_ops.end()) {
        if (*iter == p) {
            http_ops.erase(iter);
            return 0;
        }
        iter++;
    }
    msg_printf(NULL, MSG_ERROR, "HTTP operation not found");
    return ERR_NOT_FOUND;
}

int HTTP_OP_SET::nops() {
    return http_ops.size();  
}


int HTTP_OP::set_proxy(PROXY_INFO *new_pi) {
    pi.use_http_proxy = new_pi->use_http_proxy;
    strcpy(pi.http_user_name, new_pi->http_user_name);
    strcpy(pi.http_user_passwd, new_pi->http_user_passwd);
    strcpy(pi.http_server_name, new_pi->http_server_name);
    pi.http_server_port = new_pi->http_server_port;
    pi.use_http_auth = new_pi->use_http_auth;

    pi.use_socks_proxy = new_pi->use_socks_proxy;
    strcpy(pi.socks5_user_name, new_pi->socks5_user_name);
    strcpy(pi.socks5_user_passwd, new_pi->socks5_user_passwd);
    strcpy(pi.socks_server_name, new_pi->socks_server_name);
    pi.socks_server_port = new_pi->socks_server_port;
    pi.socks_version = new_pi->socks_version;

    return 0;
}

size_t libcurl_write(void *ptr, size_t size, size_t nmemb, HTTP_OP* phop) {
    // take the stream param as a FILE* and write to disk
    //CMC TODO: maybe assert stRead == size*nmemb, add exception handling on phop members
    size_t stWrite = fwrite(ptr, size, nmemb, (FILE*) phop->fileOut);
    phop->bytes_xferred += (double)(stWrite);
    //if (phop->bytes_xferred == (int) phop->content_length)
    //{ // that's all we need!
    //}
    phop->update_speed();  // this should update the transfer speed
    return stWrite;
}

size_t libcurl_read( void *ptr, size_t size, size_t nmemb, HTTP_OP* phop) {
    // OK here's the deal -- phop points to the calling object,
    // which has already pre-opened the file.  we'll want to
    // use pByte as a pointer for fseek calls into the file, and
    // write out size*nmemb # of bytes to ptr

    // take the stream param as a FILE* and write to disk
    //if (pByte) delete [] pByte;
    //pByte = new unsigned char[content_length];
    //memset(pByte, 0x00, content_length); // may as will initialize it!

    // note that fileIn was opened earlier, go to lSeek from the top and read from there
    size_t stSend = size * nmemb;
    int stRead = 0;

//    if (phop->http_op_type == HTTP_OP_POST2) {
    if (phop->req1 && ! phop->bSentHeader) { // need to send headers first, then data file
        // uck -- the way 'post2' is done, you have to read the
        // header bytes, and then cram on the file upload bytes

        // so requests from 0 to strlen(req1)-1 are from memory,
        // and from strlen(req1) to content_length are from the file

        // just send the headers from htp->req1 if needed
        if (phop->lSeek < (long) strlen(phop->req1)) {  
            // need to read header, either just starting to read (i.e.
            // this is the first time in this function for this phop)
            // or the last read didn't ask for the entire header

            stRead = strlen(phop->req1) - phop->lSeek;  // how much of header left to read

            // only memcpy if request isn't out of bounds
            if (stRead < 0) {
                stRead = 0;
            } else {
                memcpy(ptr, (void*)(phop->req1 + phop->lSeek), stRead);
            }
            phop->lSeek += (long) stRead;  // increment lSeek to new position
            phop->bytes_xferred += (double)(stRead);
            // see if we're done with headers
            phop->bSentHeader = (bool)(phop->lSeek >= (long) strlen(phop->req1));
            // reset lSeek if done to make it easier for file operations
            if (phop->bSentHeader) phop->lSeek = 0; 
            return stRead;  // 
        }
    }
    // now for file to read in (if any), also don't bother if this request
    // was just enough for the header (which was taken care of above)
    if (phop->fileIn) { 
        // note we'll have to fudge lSeek a little if there was
        // also a header, just use a temp var
        //size_t stOld = stRead; // we'll want to save the ptr location of last stRead

        // keep a separate pointer to "bump ahead" the pointer for the file data
        // as ptr may have been used above for the header info
        //unsigned char *ptr2;
        // get the file seek offset, both from the offset requested (added)
        // as well as the size of the header above discounted
        //    - ((phop->req1 && stRead>0) ? stRead : 
        //            (phop->req1 ? strlen(phop->req1) : 0L)) 
        long lFileSeek = phop->lSeek + (long) phop->file_offset;
        fseek(phop->fileIn, lFileSeek, SEEK_SET);
        if (!feof(phop->fileIn)) { // CMC TODO: better error checking for size*nmemb
            // i.e. that we don't go overbounds of the file etc, we can check against
            // content_length (which includes the strlen(req1) also)
            // note the use of stOld to get to the right position in case the header was read in above
            //ptr2 = (unsigned char*)ptr +(int)stOld;
            stRead = fread(ptr, 1, stSend, phop->fileIn); 
        }    
        phop->lSeek += (long) stRead;  // increment lSeek to new position
        phop->bytes_xferred += (double)(stRead);
    }
    phop->update_speed();  // this should update the transfer speed
    return stRead;
}

curlioerr libcurl_ioctl(CURL*, curliocmd cmd, HTTP_OP* phop) {
    // reset input stream to beginning - resends header
    // and restarts data back to starting point

    switch(cmd) {
    case CURLIOCMD_RESTARTREAD:
        phop->lSeek = 0;
        phop->bytes_xferred = phop->file_offset;
        phop->bSentHeader = false;
        break;
    default: // should never get here
        return CURLIOE_UNKNOWNCMD;
    }
    return CURLIOE_OK;
}

int libcurl_debugfunction(
    CURL*, curl_infotype type,
    unsigned char *data, size_t size, HTTP_OP* phop
) {
    const char *text;
    char hdr[100];
    char buf[1024];
    size_t mysize;
    
    SCOPE_MSG_LOG scope_messages(log_messages, CLIENT_MSG_LOG::DEBUG_NET_XFER);

    switch (type) {
    case CURLINFO_TEXT:
        scope_messages.printf("[ID#%i] info: %s\n", phop->trace_id, data );
        return 0;
    case CURLINFO_HEADER_OUT:
        text = "Sent header to server:";
        break;
    case CURLINFO_HEADER_IN:
        text = "Received header from server:";
        break;
    default: /* in case a new one is introduced to shock us */
       return 0;
    }

    sprintf( hdr,"[ID#%i] %s", phop->trace_id, text);
    mysize = min(size, sizeof(buf)-1);
    strncpy(buf, (char *)data, mysize);
    buf[mysize]='\0';
    scope_messages.printf("%s %s\n", hdr, buf);
    return 0;
}


void HTTP_OP::setupProxyCurl() {
    // CMC: use the libcurl proxy routines with this object's proxy information struct 
    /* PROXY_INFO pi useful members:
        pi.http_server_name
        pi.http_server_port
        pi.http_user_name
        pi.http_user_passwd
        pi.socks5_user_name
        pi.socks5_user_passwd
        pi.socks_server_name
        pi.socks_server_port
        pi.socks_version
        pi.use_http_auth
        pi.use_http_proxy
        pi.use_socks_proxy

        Curl self-explanatory setopt params for proxies:
            CURLOPT_HTTPPROXYTUNNEL
            CURLOPT_PROXYTYPE  (pass in CURLPROXY_HTTP or CURLPROXY_SOCKS5)
            CURLOPT_PROXYPORT  -- a long port #
            CURLOPT_PROXY - pass in char* of the proxy url
            CURLOPT_PROXYUSERPWD -- a char* in the format username:password
            CURLOPT_HTTPAUTH -- pass in one of CURLAUTH_BASIC, CURLAUTH_DIGEST, 
                CURLAUTH_GSSNEGOTIATE, CURLAUTH_NTLM, CURLAUTH_ANY, CURLAUTH_ANYSAFE
            CURLOPT_PROXYAUTH -- "or" | the above bitmasks -- only basic, digest, ntlm work
            
        */

    CURLcode curlErr;

    // CMC Note: the string szCurlProxyUserPwd must remain in memory
    // outside of this method (libcurl relies on it later when it makes
    // the proxy connection), so it has been placed as a member data for HTTP_OP
    memset(szCurlProxyUserPwd,0x00,128);

    if (pi.use_http_proxy) {
        // setup a basic http proxy
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYPORT, (long) pi.http_server_port);
        curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXY, (char*) pi.http_server_name);

        if (pi.use_http_auth) {
/* testing!
            fprintf(stdout, "Using httpauth for proxy: %s:%d %s:%s\n",
                pi.http_server_name, pi.http_server_port,
                pi.http_user_name, pi.http_user_passwd);
*/
            auth_flag = true;
            if (auth_type) {
                curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, auth_type);
            } else {
                curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_ANY & ~CURLAUTH_NTLM);
            }       
            sprintf(szCurlProxyUserPwd, "%s:%s", pi.http_user_name, pi.http_user_passwd);
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYUSERPWD, szCurlProxyUserPwd);
        }
    } else {    
        if (pi.use_socks_proxy) {
            //pi.socks_version -- picks between socks5 & socks4 -- but libcurl only socks5!
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYPORT, (long) pi.socks_server_port);
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXY, (char*) pi.socks_server_name);
            // libcurl uses blocking sockets with socks proxy, so limit timeout.
            // - imlemented with local patch to libcurl
            curlErr = curl_easy_setopt(curlEasy, CURLOPT_CONNECTTIMEOUT, 20L);

            if (
                strlen(pi.socks5_user_passwd)>0 || strlen(pi.socks5_user_name)>0
            ) {
                auth_flag = false;
                sprintf(szCurlProxyUserPwd, "%s:%s", pi.socks5_user_name, pi.socks5_user_passwd);
                curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYUSERPWD, szCurlProxyUserPwd);
                curlErr = curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_ANY & ~CURLAUTH_NTLM);
            }
        }
    }
}

const char *BOINC_RCSID_57f273bb60 = "$Id$";
