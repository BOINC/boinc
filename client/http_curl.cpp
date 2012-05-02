// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#include "cpp.h"

#ifdef _WIN32
#include "boinc_win.h"
#ifdef _MSC_VER
#define unlink _unlink
#define chdir _chdir
#endif
#else
#include "config.h"
#include <cstring>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
#include <unistd.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#endif

#include "base64.h"
#include "error_numbers.h"
#include "filesys.h"
#include "str_util.h"
#include "str_replace.h"
#include "url.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"
#include "cs_proxy.h"
#include "file_names.h"
#include "log_flags.h"
#include "network.h"
#include "net_stats.h"
#include "project.h"

#include "http_curl.h"

using std::min;
using std::vector;

static CURLM* g_curlMulti = NULL;
static char g_user_agent_string[256] = {""};
static const char g_content_type[] = {"Content-Type: application/x-www-form-urlencoded"};
static unsigned int g_trace_count = 0;

char* get_user_agent_string() {
    sprintf(g_user_agent_string, "BOINC client (%s %d.%d.%d)",
        gstate.get_primary_platform(),
        BOINC_MAJOR_VERSION, BOINC_MINOR_VERSION, BOINC_RELEASE
    );
    return (char*)&g_user_agent_string;
}

size_t libcurl_write(void *ptr, size_t size, size_t nmemb, HTTP_OP* phop) {
    // take the stream param as a FILE* and write to disk
    // TODO: maybe assert stRead == size*nmemb,
    // add exception handling on phop members
    //
    size_t stWrite = fwrite(ptr, size, nmemb, phop->fileOut);
    if (log_flags.http_xfer_debug) {
        msg_printf(NULL, MSG_INFO,
            "[http_xfer] [ID#%d] HTTP: wrote %d bytes", phop->trace_id, (int)stWrite
        );
    }
    phop->bytes_xferred += (double)(stWrite);
    phop->update_speed();  // this should update the transfer speed
    daily_xfer_history.add(stWrite, false);
    return stWrite;
}

size_t libcurl_read(void *ptr, size_t size, size_t nmemb, HTTP_OP* phop) {
    // OK here's the deal -- phop points to the calling object,
    // which has already pre-opened the file.  we'll want to
    // use pByte as a pointer for fseek calls into the file, and
    // write out size*nmemb # of bytes to ptr

    // take the stream param as a FILE* and write to disk
    // if (pByte) delete [] pByte;
    // pByte = new unsigned char[content_length];
    // memset(pByte, 0x00, content_length); // may as will initialize it!

    // note that fileIn was opened earlier,
    // go to lSeek from the top and read from there
    //
    size_t stSend = size * nmemb;
    int stRead = 0;

    if (phop->req1 && !phop->bSentHeader) {
        // need to send headers first, then data file
        // so requests from 0 to strlen(req1)-1 are from memory,
        // and from strlen(req1) to content_length are from the file
        if (phop->lSeek < (long) strlen(phop->req1)) {
            // need to read header, either just starting to read
            // (i.e. this is the first time in this function for this phop)
            // or the last read didn't ask for the entire header

            stRead = (int)strlen(phop->req1) - phop->lSeek;
                // how much of header left to read

            // only memcpy if request isn't out of bounds
            if (stRead < 0) {
                stRead = 0;
            } else {
                memcpy(ptr, (void*)(phop->req1 + phop->lSeek), stRead);
            }
            phop->lSeek += (long) stRead;  // increment lSeek to new position

            // Don't count header in bytes transferred.
            // Otherwise the GUI will show e.g. "400 out of 300 bytes xferred"
            //phop->bytes_xferred += (double)(stRead);
            daily_xfer_history.add(stRead, true);

            // see if we're done with headers
            if (phop->lSeek >= (long) strlen(phop->req1)) {
                phop->bSentHeader = true;
                phop->lSeek = 0;
            }
            return stRead;
        } else {
            // shouldn't happen
            phop->bSentHeader = true;
            phop->lSeek = 0;
        }
    }
    if (phop->fileIn) {
        long lFileSeek = phop->lSeek + (long) phop->file_offset;
        fseek(phop->fileIn, lFileSeek, SEEK_SET);
        if (!feof(phop->fileIn)) {
            stRead = (int)fread(ptr, 1, stSend, phop->fileIn);
        }
        phop->lSeek += (long) stRead;
        phop->bytes_xferred += (double)(stRead);
        daily_xfer_history.add(stRead, true);
    }
    phop->update_speed();
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

void libcurl_logdebug(
    HTTP_OP* phop, const char* desc, char *data, size_t size
) {
    char hdr[256];
    char buf[2048], *p = buf;
    size_t copy_size;

    sprintf(hdr, "[ID#%d] %s", phop->trace_id, desc);

    copy_size = min(size, sizeof(buf)-1);
    strncpy(buf, data, copy_size);
    buf[copy_size]='\0';

    p = strtok(buf, "\n");
    while(p) {
        if (log_flags.http_debug) {
            msg_printf(phop->project, MSG_INFO,
                "[http] %s %s\n", hdr, p
            );
        }
        p = strtok(NULL, "\n");
    }
}

int libcurl_debugfunction(
    CURL*, curl_infotype type, char *data, size_t size, HTTP_OP* phop
) {
    const char* desc = NULL;
    switch (type) {
    case CURLINFO_TEXT:
        desc = "Info: ";
        break;
    case CURLINFO_HEADER_OUT:
        desc = "Sent header to server:";
        break;
    case CURLINFO_HEADER_IN:
        desc = "Received header from server:";
        break;
    default: /* in case a new one is introduced to shock us */
       return 0;
    }
    libcurl_logdebug(phop, desc, data, size);
    return 0;
}

void HTTP_OP::init(PROJECT* p) {
    reset();
    start_time = gstate.now;
    start_bytes_xferred = 0;
    project = p;
}

void HTTP_OP::reset() {
    req1 = NULL;
    strcpy(infile, "");
    strcpy(outfile, "");
    strcpy(error_msg, "");
    CurlResult = CURLE_OK;
    bTempOutfile = true;
    want_download = false;
    want_upload = false;
    fileIn = NULL;
    fileOut = NULL;
    connect_error = 0;
    bytes_xferred = 0;
    bSentHeader = false;
    project = 0;
    close_socket();
}


HTTP_OP::HTTP_OP() {
    strcpy(m_url, "");
    strcpy(m_curl_ca_bundle_location, "");
    content_length = 0;
    file_offset = 0;
    strcpy(request_header, "");
    http_op_state = HTTP_STATE_IDLE;
    http_op_type = HTTP_OP_NONE;
    http_op_retval = 0;
    trace_id = g_trace_count++;
    pcurlList = NULL; // these have to be NULL, just in constructor
    curlEasy = NULL;
    pcurlFormStart = NULL;
    pcurlFormEnd = NULL;
    pByte = NULL;
    lSeek = 0;
    xfer_speed = 0;
    is_background = false;
    reset();
}

HTTP_OP::~HTTP_OP() {
    close_socket();
    close_file();
}

// Initialize HTTP GET operation;
// output goes to the given file, starting at given offset
//
int HTTP_OP::init_get(
    PROJECT* p, const char* url, const char* out, bool del_old_file, double off
) {
    if (del_old_file) {
        unlink(out);
    }
    req1 = NULL;  // not using req1, but init_post2 uses it
    file_offset = off;
    HTTP_OP::init(p);
    // usually have an outfile on a get
    if (off != 0) {
        bytes_xferred = off;
        start_bytes_xferred = off;
    }
    http_op_type = HTTP_OP_GET;
    http_op_state = HTTP_STATE_CONNECTING;
    if (log_flags.http_debug) {
        msg_printf(project, MSG_INFO, "[http] HTTP_OP::init_get(): %s", url);
    }
    return HTTP_OP::libcurl_exec(url, NULL, out, off, false);
}

// Initialize HTTP POST operation where
// the input is a file, and the output is a file,
// and both are read/written from the beginning (no resumption of partial ops)
// This is used for scheduler requests and account mgr RPCs.
//
int HTTP_OP::init_post(
    PROJECT* p, const char* url, const char* in, const char* out
) {
    int retval;
    double size;

    req1 = NULL;  // not using req1, but init_post2 uses it

    if (in) {
        strcpy(infile, in);
        retval = file_size(infile, size);
        if (retval) return retval;  // this will return 0 or ERR_NOT_FOUND
        content_length = (int)size;
    }
    HTTP_OP::init(p);
    http_op_type = HTTP_OP_POST;
    http_op_state = HTTP_STATE_CONNECTING;
    if (log_flags.http_debug) {
        msg_printf(project, MSG_INFO, "[http] HTTP_OP::init_post(): %s", url);
    }
    return HTTP_OP::libcurl_exec(url, in, out, 0.0, true);
}

// Initialize an HTTP POST operation,
// where the input is a memory string (r1) followed by an optional file (in)
// with optional offset,
// and the output goes to memory (also r1, limited by r1_len)
// This is used for file upload (both get_file_size and file_upload)
//
int HTTP_OP::init_post2(
    PROJECT* p, const char* url, char* r1, int r1_len, const char* in, double offset
) {
    int retval;
    double size;

    init(p);
    req1 = r1;
    req1_len = r1_len;
    content_length = 0;
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
    content_length += (int)strlen(req1);
    http_op_type = HTTP_OP_POST2;
    http_op_state = HTTP_STATE_CONNECTING;
    return HTTP_OP::libcurl_exec(url, in, NULL, offset, true);
}

// is URL in proxy exception list?
//
bool HTTP_OP::no_proxy_for_url(const char* url) {
    PARSED_URL purl, purl2;
    char noproxy[256];

    if (log_flags.proxy_debug) {
        msg_printf(0, MSG_INFO, "[proxy] HTTP_OP::no_proxy_for_url(): %s", url);
    }

    parse_url(url, purl);

    // tokenize the noproxy-entry and check for identical hosts
    //
    strcpy(noproxy, working_proxy_info.noproxy_hosts);
    char* token = strtok(noproxy, ",");
    while (token != NULL) {
        // extract the host from the no_proxy url
        parse_url(token, purl2);
        if (!strcmp(purl.host, purl2.host)) {
            if (log_flags.proxy_debug) {
                msg_printf(0, MSG_INFO, "[proxy] disabling proxy for %s", url);
            }
            return true;
        }
        token = strtok(NULL, ",");
    }
    if (log_flags.proxy_debug) {
        msg_printf(0, MSG_INFO, "[proxy] returning false");
    }
    return false;
}

// the following will do an HTTP GET or POST using libcurl
//
int HTTP_OP::libcurl_exec(
    const char* url, const char* in, const char* out, double offset, bool bPost
) {
    CURLMcode curlMErr;
    char strTmp[128];
    static int outfile_seqno=0;

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
        // always want an outfile for the server response, delete when op done
        bTempOutfile = true;
        sprintf(outfile, "http_temp_%d", outfile_seqno++);
    }

    curlEasy = curl_easy_init(); // get a curl_easy handle to use
    if (!curlEasy) {
        if (log_flags.http_debug) {
            msg_printf(project, MSG_INFO, "Couldn't create curlEasy handle");
        }
        return ERR_HTTP_TRANSIENT; // returns 0 (CURLM_OK) on successful handle creation
    }

    // the following seems to be a no-op
    // curl_easy_setopt(curlEasy, CURLOPT_ERRORBUFFER, error_msg);

    char esc_url[1024];
    string_substitute(m_url, esc_url, sizeof(esc_url), " ", "%20");
    curl_easy_setopt(curlEasy, CURLOPT_URL, esc_url);

    // This option determines whether curl verifies that the server
    // claims to be who you want it to be.
    // When negotiating an SSL connection,
    // the server sends a certificate indicating its identity.
    // When CURLOPT_SSL_VERIFYHOST is 2,
    // that certificate must indicate that the server is the server
    // to which you meant to connect, or the connection fails.
    // Curl considers the server the intended one when the
    // Common Name field or a Subject Alternate Name field in the certificate
    // matches the host name in the URL to which you told Curl to connect.
    // When the value is 1, the certificate must contain a Common Name field,
    // but it doesn't matter what name it says.
    // (This is not ordinarily a useful setting).
    // When the value is 0, the connection succeeds
    // regardless of the names in the certificate.
    // The default, since 7.10, is 2.
    // The checking this option controls is of the identity that
    // the server claims. The server could be lying.
    // To control lying, see CURLOPT_SSL_VERIFYPEER.
    //
    curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYHOST, 2L);

    // the following sets "tough" certificate checking
    // (i.e. whether self-signed is OK)
    // if zero below, will accept self-signed certificates
    // (cert not 3rd party trusted)
    // if non-zero below, you need a valid 3rd party CA (i.e. Verisign, Thawte)
    //
    curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYPEER, 1L);

    // if the above is nonzero, you need the following:
    //
#ifdef _WIN32
    if (strlen(m_curl_ca_bundle_location) == 0) {
        TCHAR szPath[MAX_PATH-1];
        GetModuleFileName(NULL, szPath, (sizeof(szPath)/sizeof(TCHAR)));

        TCHAR *pszProg = strrchr(szPath, '\\');
        if (pszProg) {
            szPath[pszProg - szPath + 1] = 0;

            strncat(
                m_curl_ca_bundle_location,
                szPath,
                sizeof(m_curl_ca_bundle_location)-strlen(m_curl_ca_bundle_location)
            );
            strncat(
                m_curl_ca_bundle_location,
                CA_BUNDLE_FILENAME,
                sizeof(m_curl_ca_bundle_location)-strlen(m_curl_ca_bundle_location)
            );

            if (log_flags.http_debug) {
                msg_printf(
                    project,
                    MSG_INFO,
                    "[http] HTTP_OP::libcurl_exec(): ca-bundle '%s'",
                    m_curl_ca_bundle_location
                );
            }
        }
    }
    if (boinc_file_exists(m_curl_ca_bundle_location)) {
        // call this only if a local copy of ca-bundle.crt exists;
        // otherwise, let's hope that it exists in the default place
        //
        curl_easy_setopt(curlEasy, CURLOPT_CAINFO, m_curl_ca_bundle_location);
        if (log_flags.http_debug) {
            msg_printf(
                project,
                MSG_INFO,
                "[http] HTTP_OP::libcurl_exec(): ca-bundle set"
            );
        }
    }
#else
    if (boinc_file_exists(CA_BUNDLE_FILENAME)) {
        // call this only if a local copy of ca-bundle.crt exists;
        // otherwise, let's hope that it exists in the default place
        //
        curl_easy_setopt(curlEasy, CURLOPT_CAINFO, CA_BUNDLE_FILENAME);
    }
#endif

    // set the user agent as this boinc client & version
    //
    curl_easy_setopt(curlEasy, CURLOPT_USERAGENT, g_user_agent_string);

    // bypass any signal handlers that curl may want to install
    //
    curl_easy_setopt(curlEasy, CURLOPT_NOSIGNAL, 1L);
    // bypass progress meter
    //
    curl_easy_setopt(curlEasy, CURLOPT_NOPROGRESS, 1L);

    // setup timeouts
    //
    curl_easy_setopt(curlEasy, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(curlEasy, CURLOPT_LOW_SPEED_LIMIT, config.http_transfer_timeout_bps);
    curl_easy_setopt(curlEasy, CURLOPT_LOW_SPEED_TIME, config.http_transfer_timeout);
    curl_easy_setopt(curlEasy, CURLOPT_CONNECTTIMEOUT, 120L);

    // force curl to use HTTP/1.0 if config specifies it
    // (curl uses 1.1 by default)
    //
    if (config.http_1_0 || (config.force_auth == "ntlm")) {
        curl_easy_setopt(curlEasy, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
    }
    curl_easy_setopt(curlEasy, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curlEasy, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curlEasy, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curlEasy, CURLOPT_POST301, 1L);

    // if we tell Curl to accept any encoding (e.g. deflate)
    // it seems to accept them all, which screws up projects that
    // use gzip at the application level.
    // So, detect this and don't accept any encoding in that case
    //
    // Per: http://curl.haxx.se/dev/readme-encoding.html
    // NULL disables, empty string accepts all.
    if (out) {
        if (ends_with(out, ".gzt")) {
            curl_easy_setopt(curlEasy, CURLOPT_ENCODING, NULL);
        } else if (!ends_with(out, ".gz")) {
            curl_easy_setopt(curlEasy, CURLOPT_ENCODING, "");
        }
    } else {
        curl_easy_setopt(curlEasy, CURLOPT_ENCODING, "");
    }

    // setup any proxy they may need
    //
    setup_proxy_session(no_proxy_for_url(url));


    // set the content type in the header
    //
    pcurlList = curl_slist_append(pcurlList, g_content_type);

    // set the file offset for resumable downloads
    //
    if (!bPost && offset>0.0f) {
        file_offset = offset;
        sprintf(strTmp, "Range: bytes=%.0f-", offset);
        pcurlList = curl_slist_append(pcurlList, strTmp);
    }

    // set up an output file for the reply
    //
    if (strlen(outfile)) {
        if (file_offset>0.0) {
            fileOut = boinc_fopen(outfile, "ab+");
        } else {
            fileOut = boinc_fopen(outfile, "wb+");
        }
        if (!fileOut) {
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "Can't create HTTP response output file %s", outfile
            );
            http_op_retval = ERR_FOPEN;
            http_op_state = HTTP_STATE_DONE;
            return ERR_FOPEN;
        }
        // we can make the libcurl_write "fancier" in the future,
        // for now it just fwrite's to the file request, which is sufficient
        //
        curl_easy_setopt(curlEasy, CURLOPT_WRITEFUNCTION, libcurl_write);
        // note that in my lib_write I'm sending in a pointer
        // to this instance of HTTP_OP
        //
        curl_easy_setopt(curlEasy, CURLOPT_WRITEDATA, this);
    }

    if (bPost) {
        want_upload = true;
        want_download = false;
        if (infile && strlen(infile)>0) {
            fileIn = boinc_fopen(infile, "rb");
            if (!fileIn) {
                msg_printf(NULL, MSG_INTERNAL_ERROR, "No HTTP input file %s", infile);
                http_op_retval = ERR_FOPEN;
                http_op_state = HTTP_STATE_DONE;
                return ERR_FOPEN;
            }
        }

        if (pcurlList) { // send custom headers if required
            curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, pcurlList);
        }

        // set the data file info to read for the PUT/POST
        // note the use of this curl typedef for large filesizes

#if 0
        // HTTP PUT method
        curl_off_t fs = (curl_off_t) content_length;
        curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(curlEasy, CURLOPT_INFILESIZE, content_length);
        curl_easy_setopt(curlEasy, CURLOPT_READDATA, fileIn);
        curl_easy_setopt(curlEasy, CURLOPT_INFILESIZE_LARGE, fs);
        curl_easy_setopt(curlEasy, CURLOPT_PUT, 1L);
#endif

        // HTTP POST method
        // set the multipart form for the file --
        // boinc just has the one section (file)

#if 0
        //  if we ever want to do POST as multipart forms someday
        // (many seem to prefer it that way, i.e. libcurl)
        //
        pcurlFormStart = pcurlFormEnd = NULL;
        curl_formadd(&pcurlFormStart, &pcurlFormEnd,
           CURLFORM_FILECONTENT, infile,
           CURLFORM_CONTENTSLENGTH, content_length,
           CURLFORM_CONTENTTYPE, g_content_type,
           CURLFORM_END
        );
        curl_formadd(&post, &last,
           CURLFORM_COPYNAME, "logotype-image",
           CURLFORM_FILECONTENT, "curl.png", CURLFORM_END
        );
        curl_easy_setopt(curlEasy, CURLOPT_HTTPPOST, pcurlFormStart);
#endif

        curl_off_t fs = (curl_off_t) content_length;

        pByte = NULL;
        lSeek = 0;    // initialize the vars we're going to use for byte transfers

        // we can make the libcurl_read "fancier" in the future,
        // for now it just fwrite's to the file request, which is sufficient
        //
        curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDS, NULL);
        curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDSIZE_LARGE, fs);
        curl_easy_setopt(curlEasy, CURLOPT_READFUNCTION, libcurl_read);
        // in my lib_write I'm sending in a pointer to this instance of HTTP_OP
        //
        curl_easy_setopt(curlEasy, CURLOPT_READDATA, this);

        // callback function to rewind input file
        //
        curl_easy_setopt(curlEasy, CURLOPT_IOCTLFUNCTION, libcurl_ioctl);
        curl_easy_setopt(curlEasy, CURLOPT_IOCTLDATA, this);

        curl_easy_setopt(curlEasy, CURLOPT_POST, 1L);
    } else {  // GET
        want_upload = false;
        want_download = true;

        // now write the header, pcurlList gets freed in net_xfer_curl
        //
        if (pcurlList) { // send custom headers if required
            curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, pcurlList);
        }

        // setup the GET!
        //
        curl_easy_setopt(curlEasy, CURLOPT_HTTPGET, 1L);
    }

#ifdef __APPLE__
    // cURL 7.19.7 with c-ares 1.7.0 did not fall back to IPv4 when IPv6 
    // DNS lookup failed on Macs with certain default settings if connected 
    // to the Internet by an AT&T U-Verse 2-Wire Gateway.  This work-around 
    // may not be needed any more for cURL 7.21.7, but keep it to be safe.
    curl_easy_setopt(curlEasy, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
#endif

    // turn on debug info if tracing enabled
    //
    if (log_flags.http_debug) {
        curl_easy_setopt(curlEasy, CURLOPT_DEBUGFUNCTION, libcurl_debugfunction);
        curl_easy_setopt(curlEasy, CURLOPT_DEBUGDATA, this );
        curl_easy_setopt(curlEasy, CURLOPT_VERBOSE, 1L);
    }

    // last but not least, add this to the curl_multi

    curlMErr = curl_multi_add_handle(g_curlMulti, curlEasy);
    if (curlMErr != CURLM_OK && curlMErr != CURLM_CALL_MULTI_PERFORM) {
        // bad error, couldn't attach easy curl handle
        msg_printf(0, MSG_INTERNAL_ERROR,
            "Couldn't add curlEasy handle to curlMulti"
        );
        return ERR_HTTP_TRANSIENT;
        // returns 0 (CURLM_OK) on successful handle creation
    }

    return 0;
}

// Returns true if the HTTP operation is complete
//
bool HTTP_OP::http_op_done() {
    return (http_op_state == HTTP_STATE_DONE);
}

HTTP_OP_SET::HTTP_OP_SET() {
    bytes_up = 0;
    bytes_down = 0;
}

// Adds an HTTP_OP to the set
//
void HTTP_OP_SET::insert(HTTP_OP* ho) {
    http_ops.push_back(ho);
}

// Remove an HTTP_OP from the set
//
int HTTP_OP_SET::remove(HTTP_OP* p) {
    vector<HTTP_OP*>::iterator iter;

    iter = http_ops.begin();
    while (iter != http_ops.end()) {
        if (*iter == p) {
            iter = http_ops.erase(iter);
            return 0;
        }
        iter++;
    }
    return ERR_NOT_FOUND;
}

int HTTP_OP_SET::nops() {
    return (int)http_ops.size();
}

// Curl self-explanatory setopt params for proxies:
//    CURLOPT_HTTPPROXYTUNNEL
//    CURLOPT_PROXYTYPE  (pass in CURLPROXY_HTTP or CURLPROXY_SOCKS5)
//    CURLOPT_PROXYPORT  -- a long port #
//    CURLOPT_PROXY - pass in char* of the proxy url
//    CURLOPT_PROXYUSERPWD -- a char* in the format username:password
//    CURLOPT_HTTPAUTH -- pass in one of CURLAUTH_BASIC, CURLAUTH_DIGEST,
//        CURLAUTH_GSSNEGOTIATE, CURLAUTH_NTLM, CURLAUTH_ANY, CURLAUTH_ANYSAFE
//    CURLOPT_PROXYAUTH -- "or" | the above bitmasks -- only basic, digest, ntlm work
void HTTP_OP::setup_proxy_session(bool no_proxy) {

    // CMC Note: the string m_curl_user_credentials must remain in memory
    // outside of this method (libcurl relies on it later when it makes
    // the proxy connection), so it has been placed as a member data for HTTP_OP
    //
    strcpy(m_curl_user_credentials, "");

    if (no_proxy) {
        curl_easy_setopt(curlEasy, CURLOPT_PROXY, "");
        return;
    }

    pi = working_proxy_info;
    if (pi.use_http_proxy) {
        if (log_flags.proxy_debug) {
            msg_printf(
                0, MSG_INFO, "[proxy]: setting up proxy %s:%d",
                pi.http_server_name, pi.http_server_port
            );
        }

        // setup a basic http proxy
        curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
        curl_easy_setopt(curlEasy, CURLOPT_PROXYPORT, (long) pi.http_server_port);
        curl_easy_setopt(curlEasy, CURLOPT_PROXY, (char*) pi.http_server_name);

        if (pi.use_http_auth) {
            if (config.force_auth == "basic") {
                curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_BASIC);
            } else if (config.force_auth == "digest") {
                curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_DIGEST);
            } else if (config.force_auth == "gss-negotiate") {
                curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_GSSNEGOTIATE);
            } else if (config.force_auth == "ntlm") {
                curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_NTLM);
            } else {
                curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
            }
            sprintf(m_curl_user_credentials, "%s:%s", pi.http_user_name, pi.http_user_passwd);
            curl_easy_setopt(curlEasy, CURLOPT_PROXYUSERPWD, m_curl_user_credentials);
        }

    } else if (pi.use_socks_proxy) {
        // CURL only supports SOCKS version 5
        curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
        curl_easy_setopt(curlEasy, CURLOPT_PROXYPORT, (long) pi.socks_server_port);
        curl_easy_setopt(curlEasy, CURLOPT_PROXY, (char*) pi.socks_server_name);
        // libcurl uses blocking sockets with socks proxy, so limit timeout.
        // - imlemented with local patch to libcurl
        curl_easy_setopt(curlEasy, CURLOPT_CONNECTTIMEOUT, 20L);

        if (
            strlen(pi.socks5_user_passwd) || strlen(pi.socks5_user_name)
        ) {
            sprintf(m_curl_user_credentials, "%s:%s", pi.socks5_user_name, pi.socks5_user_passwd);
            curl_easy_setopt(curlEasy, CURLOPT_PROXYUSERPWD, m_curl_user_credentials);
            curl_easy_setopt(curlEasy, CURLOPT_PROXYAUTH, CURLAUTH_ANY & ~CURLAUTH_NTLM);
        }
    } else if (pi.have_autodetect_proxy_settings && strlen(pi.autodetect_server_name)) {
        if (log_flags.proxy_debug) {
            msg_printf(0, MSG_INFO,
                "[proxy] HTTP_OP::setup_proxy_session(): setting up automatic proxy %s:%d",
                pi.autodetect_server_name, pi.autodetect_port
            );
        }

        switch(pi.autodetect_protocol) {
        case URL_PROTOCOL_SOCKS:
            curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
            break;
        case URL_PROTOCOL_HTTP:
        case URL_PROTOCOL_HTTPS:
        default:
            curl_easy_setopt(curlEasy, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
            break;
        }
        curl_easy_setopt(curlEasy, CURLOPT_PROXYPORT, (long) pi.autodetect_port);
        curl_easy_setopt(curlEasy, CURLOPT_PROXY, (char*) pi.autodetect_server_name);
    }
}


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
    curl_global_cleanup();
    return 0;
}

void HTTP_OP::close_socket() {
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

void HTTP_OP::close_file() {
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

void HTTP_OP_SET::get_fdset(FDSET_GROUP& fg) {
    curl_multi_fdset(
        g_curlMulti, &fg.read_fds, &fg.write_fds, &fg.exc_fds, &fg.max_fd
    );
}

// we have a message for this HTTP_OP.
// get the response code for this request
//
void HTTP_OP::handle_messages(CURLMsg *pcurlMsg) {
    int retval;

    curl_easy_getinfo(curlEasy,
        CURLINFO_RESPONSE_CODE, &response
    );

    curl_easy_getinfo(curlEasy,
        CURLINFO_OS_ERRNO, &connect_error
    );

    // update byte counts and transfer speed
    //
    if (want_download) {
        // SIZE_DOWNLOAD is the byte count "on the wire"
        // (possible with compression)
        // TOTAL_TIME is the elapsed time of the download
        // STARTTRANSFER_TIME is portion of elapsed time involved
        // with setup (connection establishment etc.)
        // SPEED_DOWNLOAD is bytes/sec based on uncompressed size
        // (we don't use it)
        //
        double size_download, total_time, starttransfer_time;
        curl_easy_getinfo(curlEasy, CURLINFO_SIZE_DOWNLOAD, &size_download);
        curl_easy_getinfo(curlEasy, CURLINFO_TOTAL_TIME, &total_time);
        curl_easy_getinfo(curlEasy,
            CURLINFO_STARTTRANSFER_TIME, &starttransfer_time
        );
        double dt = total_time - starttransfer_time;
        if (dt > 0) {
            gstate.net_stats.down.update(size_download, dt);
        }
    }
    if (want_upload) {
        double size_upload, total_time, starttransfer_time;
        curl_easy_getinfo(curlEasy, CURLINFO_SIZE_UPLOAD, &size_upload);
        curl_easy_getinfo(curlEasy, CURLINFO_TOTAL_TIME, &total_time);
        curl_easy_getinfo(curlEasy,
            CURLINFO_STARTTRANSFER_TIME, &starttransfer_time
        );
        double dt = total_time - starttransfer_time;
        if (dt > 0) {
            gstate.net_stats.up.update(size_upload, dt);
        }
    }

    // the op is done if curl_multi_msg_read gave us a msg for this http_op
    //
    http_op_state = HTTP_STATE_DONE;
    CurlResult = pcurlMsg->data.result;

    if (CurlResult == CURLE_OK) {
        if ((response/100)*100 == HTTP_STATUS_OK) {
            http_op_retval = 0;
        } else if ((response/100)*100 == HTTP_STATUS_CONTINUE) {
            return;
        } else {
            // Got a response from server but its not OK or CONTINUE,
            // so save response with error message to display later.
            //
            if (response >= 400) {
                strcpy(error_msg, boincerror(response));
            } else {
                sprintf(error_msg, "HTTP error %ld", response);
            }
            switch (response) {
            case HTTP_STATUS_NOT_FOUND:
            case HTTP_STATUS_RANGE_REQUEST_ERROR:
                http_op_retval = ERR_HTTP_PERMANENT;
                break;
            default:
                http_op_retval = ERR_HTTP_TRANSIENT;
            }
        }
        net_status.http_op_succeeded();
    } else {
        strcpy(error_msg, curl_easy_strerror(CurlResult));
        switch(CurlResult) {
        case CURLE_COULDNT_RESOLVE_HOST:
            reset_dns();
            http_op_retval = ERR_GETHOSTBYNAME;
            break;
        case CURLE_COULDNT_CONNECT:
            http_op_retval = ERR_CONNECT;
            break;
        default:
            http_op_retval = ERR_HTTP_TRANSIENT;
        }

        // trigger a check for whether we're connected,
        // but not if this is a background operation
        //
        if (!is_background) {
            net_status.got_http_error();
        }
        if (log_flags.http_debug) {
            msg_printf(project, MSG_INFO,
                "[http] HTTP error: %s", error_msg
            );
        }
    }

    if (!http_op_retval && http_op_type == HTTP_OP_POST2) {
        // for a successfully completed request on a "post2" --
        // read in the temp file into req1 memory
        //
        size_t dSize = ftell(fileOut);
        retval = fseek(fileOut, 0, SEEK_SET);
        if (retval) {
            // flag as a bad response for a possible retry later
            response = 1;
            msg_printf(NULL, MSG_INTERNAL_ERROR,
                "[http] can't rewind post output file %s",
                outfile
            );
        } else {
            strcpy(req1, "");
            if (dSize >= (size_t)req1_len) {
                dSize = req1_len-1;
            }
            size_t nread = fread(req1, 1, dSize, fileOut);
            if (nread != dSize) {
                if (log_flags.http_debug) {
                    msg_printf(project, MSG_INFO,
                        "[http] post output file read failed %d",
                        (int)nread
                    );
                }
            }
            req1[nread] = 0;
        }
    }

    // close files and "sockets" (i.e. libcurl handles)
    //
    close_file();
    close_socket();

    // finally remove the tmpfile if not explicitly set
    //
    if (bTempOutfile) {
        boinc_delete_file(outfile);
    }
}

void HTTP_OP_SET::got_select(FDSET_GROUP&, double timeout) {
    int iNumMsg;
    HTTP_OP* hop = NULL;
    CURLMsg *pcurlMsg = NULL;

    int iRunning = 0;  // curl flags for max # of fds & # running queries
    CURLMcode curlMErr;

    // get the data waiting for transfer in or out
    // use timeout value so that we don't hog CPU in this loop
    //
    while (1) {
        curlMErr = curl_multi_perform(g_curlMulti, &iRunning);
        if (curlMErr != CURLM_CALL_MULTI_PERFORM) break;
        if (dtime() - gstate.now > timeout) break;
    }

    // read messages from curl that may have come in from the above loop
    //
    while (1) {
        pcurlMsg = curl_multi_info_read(g_curlMulti, &iNumMsg);
        if (!pcurlMsg) break;

        // if we have a msg, then somebody finished
        // can check also with pcurlMsg->msg == CURLMSG_DONE
        //
        hop = lookup_curl(pcurlMsg->easy_handle);
        if (!hop) continue;
        hop->handle_messages(pcurlMsg);
    }
}

// Return the HTTP_OP object with given Curl object
//
HTTP_OP* HTTP_OP_SET::lookup_curl(CURL* pcurl)  {
    for (unsigned int i=0; i<http_ops.size(); i++) {
        if (http_ops[i]->curlEasy == pcurl) {
            return http_ops[i];
        }
    }
    return 0;
}

// Update the transfer speed for this HTTP_OP
// called on every I/O
//
void HTTP_OP::update_speed() {
    double delta_t = dtime() - start_time;
    if (delta_t > 0) {
        xfer_speed = (bytes_xferred-start_bytes_xferred) / delta_t;
    }
}

void HTTP_OP::set_speed_limit(bool is_upload, double bytes_sec) {
#if LIBCURL_VERSION_NUM >= 0x070f05
    CURLcode cc = CURLE_OK;
    curl_off_t bs = (curl_off_t)bytes_sec;

    if (is_upload) {
        cc = curl_easy_setopt(curlEasy, CURLOPT_MAX_SEND_SPEED_LARGE, bs);
    } else {
        cc = curl_easy_setopt(curlEasy, CURLOPT_MAX_RECV_SPEED_LARGE, bs);
    }
    if (cc && log_flags.http_debug) {
        msg_printf(project, MSG_INFO,
            "[http] Curl error in set_speed_limit(): %s",
            curl_easy_strerror(cc)
        );
    }
#endif
}

void HTTP_OP_SET::cleanup_temp_files() {
    char filename[256];

    DIRREF d = dir_open(".");
    while (1) {
        int retval = dir_scan(filename, d, sizeof(filename));
        if (retval) break;
        if (strstr(filename, "blc") != filename) continue;
        if (!is_file(filename)) continue;
        boinc_delete_file(filename);
    }
    dir_close(d);
}

