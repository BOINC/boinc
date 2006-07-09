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

// HTTP_OP represents an HTTP operation.
// There are variants for GET and POST,
// and for the data source/sink (see below).

// We use libcurl: http://curl.haxx.se/libcurl

#ifndef _HTTP_
#define _HTTP_

// SOCKS #defines
#define SOCKS_VERSION_4             0x04
#define SOCKS_VERSION_5             0x05

#include <curl/curl.h>

#include "network.h"
#include "proxy_info.h"

extern int curl_init();
extern int curl_cleanup();

// official HTTP status codes

#define HTTP_STATUS_CONTINUE                100 
#define HTTP_STATUS_OK                      200
#define HTTP_STATUS_PARTIAL_CONTENT         206
#define HTTP_STATUS_MOVED_PERM              301
#define HTTP_STATUS_MOVED_TEMP              302
#define HTTP_STATUS_NOT_FOUND               404
#define HTTP_STATUS_PROXY_AUTH_REQ          407
#define HTTP_STATUS_RANGE_REQUEST_ERROR     416
#define HTTP_STATUS_INTERNAL_SERVER_ERROR   500
#define HTTP_STATUS_SERVICE_UNAVAILABLE     503

#define HTTP_OP_NONE    0
#define HTTP_OP_GET     1
    // data sink is a file (used for file download)
#define HTTP_OP_POST    2
    // data source and sink are files (used for scheduler op)
#define HTTP_OP_HEAD    4
    // no data (used for file upload)
#define HTTP_OP_POST2   5
    // a POST operation where the request comes from a combination
    // of a string and a file w/offset,
    // and the reply goes into a memory buffer.
    // Used for file upload

#define HTTP_STATE_IDLE             0
#define HTTP_STATE_CONNECTING       1
#define HTTP_STATE_DONE             2

class HTTP_OP {
public:
    HTTP_OP();
    ~HTTP_OP();

    PROXY_INFO pi;

	char m_url[256];  
	char szCurlProxyUserPwd[128]; // string needed for proxy username/password

    int content_length;
    double file_offset;
    int trace_id;
    char request_header[4096];

    FILE* fileIn;
	FILE* fileOut;  // CMC need an output file for POST responses
	CURL* curlEasy; // the "easy curl" handle for this net_xfer request
	struct curl_slist *pcurlList; // curl slist for http headers
	struct curl_httppost *pcurlFormStart; // a pointer to a form item for POST
	struct curl_httppost *pcurlFormEnd; // a pointer to a form item for POST
	unsigned char* pByte;  // pointer to bytes for reading via libcurl_read function

	long lSeek;  // this is a pointer within the file we're reading
    char infile[256];
    char outfile[256];
    char error_msg[256];    // put Curl error message here
	bool bTempOutfile; // CMC -- flag that outfile is really a tempfile we should delete
    char* req1;
	bool bSentHeader;  // CMC -- a flag that I already sent the header
	CURLcode CurlResult;   // CMC -- send up curl result code

    bool want_download;     // at most one should be true
    bool want_upload;
    int error;
	int response;
    double start_time;
    double xfer_speed;
    double bytes_xferred;   // bytes transferred in this session

	int http_op_state;     // values above
    int http_op_type;
    int http_op_retval;

    // save authorization types supported by proxy/socks server
    bool auth_flag;       // TRUE = server uses authorization
    long auth_type;       // 0 = haven't contacted server yet.

    void reset();
    void init();
    int get_ip_addr(int &ip_addr);
    void close_socket();
    void close_file();
    void update_speed();
    void got_error();

	//int init_head(const char* url);
    int init_get(const char* url, const char* outfile, bool del_old_file, double offset=0);
    int init_post(const char* url, const char* infile, const char* outfile);
    int init_post2(
        const char* url,
        char* req1,     // first part of request.  ALSO USED FOR REPLY
        const char* infile, double offset     // infile is NULL if no file sent
    );
    bool http_op_done();
	int set_proxy(PROXY_INFO *new_pi);
	void setupProxyCurl();

private:
	// internal use in the class -- takes an init_get/post/post2 and turns it into
	// an appropriate libcurl request
	int libcurl_exec(const char* url, const char* in = NULL, const char* out = NULL, 
		double offset = 0.0f, bool bPost = true
    );
};

// global function used by libcurl to write http replies to disk
size_t libcurl_write(void *ptr, size_t size, size_t nmemb, HTTP_OP* phop);
size_t libcurl_read( void *ptr, size_t size, size_t nmemb, HTTP_OP* phop);
curlioerr libcurl_ioctl(CURL *handle, curliocmd cmd, HTTP_OP* phop);
int libcurl_debugfunction(CURL *handle, curl_infotype type,
             unsigned char *data, size_t size, HTTP_OP* phop);

// represents a set of HTTP requests in progress
//
class HTTP_OP_SET {
    std::vector<HTTP_OP*> http_ops;
public:
    HTTP_OP_SET();
    int insert(HTTP_OP*);
    int remove(HTTP_OP*);
    int nops();

    double max_bytes_sec_up, max_bytes_sec_down;
        // user-specified limits on throughput
    double bytes_left_up, bytes_left_down;
        // bytes left to transfer in the current second
    double bytes_up, bytes_down;
        // total bytes transferred

	void get_fdset(FDSET_GROUP&);
    void got_select(FDSET_GROUP&, double);
    HTTP_OP* lookup_curl(CURL* pcurl);   // lookup by easycurl handle

};

extern void parse_url(const char* url, char* host, int &port, char* file);

#endif //__HTTP_H
