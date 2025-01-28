// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#endif

#include "str_util.h"
#include "str_replace.h"

#include "url.h"

using std::string;

// Break a URL down into its protocol, server, port and file components
// URL format:
// [{http|https|socks}://][user[:passwd]@]host.dom.dom[:port][/dir/file]
//
void parse_url(const char* url, PARSED_URL& purl) {
    char* p, *q, *buf;
    char _buf[256];

    // strip off the protocol if present
    //
    if (strncmp(url, "http://", 7) == 0) {
        safe_strcpy(_buf, url+7);
        purl.protocol = URL_PROTOCOL_HTTP;
    } else if (strncmp(url, "https://", 8) == 0) {
        safe_strcpy(_buf, url+8);
        purl.protocol = URL_PROTOCOL_HTTPS;
    } else if (strncmp(url, "socks://", 8) == 0) {
        safe_strcpy(_buf, url+8);
        purl.protocol = URL_PROTOCOL_SOCKS;
    } else {
        safe_strcpy(_buf, url);
        purl.protocol = URL_PROTOCOL_UNKNOWN;
    }
    buf = _buf;

    // parse user name and password
    //
    safe_strcpy(purl.user, "");
    safe_strcpy(purl.passwd, "");
    p = strchr(buf, '@');
    if (p) {
        *p = 0;
        q = strchr(buf, ':');
        if (q) {
            *q = 0;
            safe_strcpy(purl.user, buf);
            safe_strcpy(purl.passwd, q+1);
        } else {
            safe_strcpy(purl.user, buf);
        }
        buf = p+1;
    }

    // parse and strip off file part if present
    //
    p = strchr(buf, '/');
    if (p) {
        safe_strcpy(purl.file, p+1);
        *p = 0;
    } else {
        safe_strcpy(purl.file, "");
    }

    // parse and strip off port if present
    //
    p = strchr(buf,':');
    if (p) {
        purl.port = atol(p+1);
        *p = 0;
    } else {
        // CMC note:  if they didn't pass in a port #,
        //    but the url starts with https://, assume they
        //    want a secure port (HTTPS, port 443)
        purl.port = (purl.protocol == URL_PROTOCOL_HTTPS) ? 443 : 80;
    }

    // what remains is the host
    //
    safe_strcpy(purl.host, buf);
}

// The following functions do "URL-escaping", i.e. escaping GET arguments
// to be passed in a URL

static char x2c(char *what) {
    char digit;
    digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
    digit *= 16;
    digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
    return digit;
}

// size not really needed since unescaping can only shrink
//
void unescape_url(char *url, int url_size) {
    int x,y;
    for (x=0,y=0; url[y] && (x<url_size);++x,++y) {
        if ((url[x] = url[y]) == '%') {
            url[x] = x2c(&url[y+1]);
            y+=2;
        }
    }
    url[x] = '\0';
}

// unescape_url needs to be able to handle potentially hostile URLs
//
void unescape_url(string& url) {
    char buf[1024];
    strlcpy(buf, url.c_str(), sizeof(buf));
    unescape_url(buf, sizeof(buf));
    url = buf;
}

static void c2x(unsigned char num, char* buf) {
    char d1 = num / 16;
    char d2 = num % 16;
    int abase1 = (d1 < 10) ? 48 : 55;
    int abase2 = (d2 < 10) ? 48 : 55;
    buf[0] = d1 + abase1;
    buf[1] = d2 + abase2;
    buf[2] = 0;
}

void escape_url(const char *in, char*out, int out_size) {
    char buf[256];
    int x, y;
    for (x=0, y=0; in[x] && (y<out_size-3); ++x) {
        if (isalnum(in[x])) {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '%';
            ++y;
            out[y] = 0;
            c2x(in[x], buf);
            strlcat(out, buf, out_size);
            y += 2;
        }
    }
    out[y] = 0;
}

// escape_url needs to be able to handle potentially hostile URLs
//
void escape_url(string& url) {
    char buf[1024];
    escape_url(url.c_str(), buf, sizeof(buf));
    url = buf;
}

// Escape a project URL, cutting off the "http://",
// converting everthing other than alphanumbers, ., - and _ to "_".
// This is used as the project directory name.
// Note: does not convert to lowercase.
//
void escape_url_readable(char *in, char* out) {
    int x, y;
    char *temp;

    temp = strstr(in,"://");
    if (temp) {
        in = temp + strlen("://");
    }
    for (x=0, y=0; in[x]; ++x) {
        if (isalnum(in[x]) || in[x]=='.' || in[x]=='-' || in[x]=='_') {
            out[y] = in[x];
            ++y;
        } else {
            out[y] = '_';
            ++y;
        }
    }
    out[y] = 0;
}


// Canonicalize a master url.
//   - Convert the first part of a URL (before the "://") to http://,
// or prepend it
//   - Remove double slashes in the rest
//   - Add a trailing slash if necessary
//   - Convert all alphabet characters to lower case
//
void canonicalize_master_url(char* url, int len) {
    char buf[1024];
    size_t n;
    bool bSSL = false; // keep track if they sent in https://

    char *p = strstr(url, "://");
    if (p) {
        bSSL = (bool) (p == url + 5);
        strlcpy(buf, p+3, sizeof(buf));
    } else {
        strlcpy(buf, url, sizeof(buf));
    }
    while (1) {
        p = strstr(buf, "//");
        if (!p) break;
        strcpy_overlap(p, p+1);
    }
    n = strlen(buf);
    if (buf[n-1] != '/' && (n<sizeof(buf)-2)) {
        safe_strcat(buf, "/");
    }
    for (size_t i=0; i<n-1; i++) {
        // stop converting to lower-case, if we've reached the boundary of the domain name
        if (buf[i] == '/') break;
        buf[i] = tolower(static_cast<unsigned char>(buf[i]));
    }
    snprintf(url, len, "http%s://%s", (bSSL ? "s" : ""), buf);
    url[len-1] = 0;
}

void canonicalize_master_url(string& url) {
    char buf[1024];
    safe_strcpy(buf, url.c_str());
    canonicalize_master_url(buf, sizeof(buf));
    url = buf;
}

// return true if url1 and url2 are the same
// except ur1 is http: and url2 is https:
//
bool is_https_transition(const char* url1, const char* url2) {
    if (strstr(url1, "http://") != url1) return false;
    if (strstr(url2, "https://") != url2) return false;
    if (strcmp(url1+strlen("http://"), url2+strlen("https://"))) return false;
    return true;
}

// return true if url1 and url2 are the same except protocol
//
bool urls_match(const char* url1, const char* url2) {
    const char* p = strstr(url1, "//");
    const char* q = strstr(url2, "//");
    if (!p || !q) return false;
    return strcmp(p, q) == 0;
}

// is the string a valid master URL, in canonical form?
//
bool valid_master_url(char* buf) {
    char* p, *q;
    size_t n;
    bool bSSL = false;

    p = strstr(buf, "http://");
    if (p != buf) {
        // allow https
        p = strstr(buf, "https://");
        if (p == buf) {
            bSSL = true;
        } else {
            return false; // no http or https, it's bad!
        }
    }
    q = p+strlen(bSSL ? "https://" : "http://");
    p = strchr(q, '.');
    if (!p) return false;
    if (p == q) return false;
    q = p+1;
    p = strchr(q, '/');
    if (!p) return false;
    if (p == q) return false;
    n = strlen(buf);
    if (buf[n-1] != '/') return false;
    return true;
}

void escape_project_url(char *in, char* out) {
    escape_url_readable(in, out);
    char& last = out[strlen(out)-1];
    // remove trailing _
    if (last == '_') {
        last = '\0';
    }
}

bool is_https(const char* url) {
    return (strncmp(url, "https://", 8) == 0);
}
