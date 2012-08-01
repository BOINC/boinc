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

// A very crude interface for parsing XML files;
// assumes all elements are either single-line or
// have start and end tags on separate lines.
// This is meant to be used ONLY for parsing XML files produced
// by the BOINC scheduling server or client.
// Could replace this with a more general parser.

#include "parse.h"

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#if HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#endif

#ifdef _MSC_VER
#define strdup _strdup
#endif

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

#include "error_numbers.h"
#include "str_util.h"
#include "str_replace.h"

using std::string;

// Parse a boolean; tag is of form "foobar"
// Accept either <foobar/>, <foobar />, or <foobar>0|1</foobar>
// (possibly with leading/trailing white space)
//
bool parse_bool(const char* buf, const char* tag, bool& result) {
    char tag2[256], tag3[256];
    int x;
    // quick check to reject most cases
    //
    if (!strstr(buf, tag)) {
        return false;
    }
    sprintf(tag2, "<%s/>", tag);
    sprintf(tag3, "<%s />", tag);
    if (match_tag(buf, tag2) || match_tag(buf, tag3)) {
        result = true;
        return true;
    }
    sprintf(tag2, "<%s>", tag);
    if (parse_int(buf, tag2, x)) {
        result = (x != 0);
        return true;
    }
    return false;
}

// parse a string of the form ...<tag attrs>string</tag>...;
// returns the "string" part.
// Does XML unescaping (replace &lt; with <)
// "string" may not include '<'
// Strips white space from ends.
// Use "<tag", not "<tag>", if there might be attributes
//
bool parse_str(const char* buf, const char* tag, char* dest, int destlen) {
    string str;
    const char* p;
    int len;

    p = strstr(buf, tag);
    if (!p) return false;
    p = strchr(p, '>');
    p++;
    const char* q = strchr(p, '<');
    if (!q) return false;
    len = (int)(q-p);
    if (len >= destlen) len = destlen-1;
    memcpy(dest, p, len);
    dest[len] = 0;
    strip_whitespace(dest);
    xml_unescape(dest);
    return true;
}

bool parse_str(const char* buf, const char* tag, string& dest) {
    char tempbuf[1024];
    if (!parse_str(buf, tag, tempbuf, 1024)) return false;
    dest = tempbuf;
    return true;
}

// parse a string of the form 'xxx name="value" xxx';
// returns value in dest
//
void parse_attr(const char* buf, const char* name, char* dest, int len) {
    const char* p;
    const char *q;

    strcpy(dest, "");
    p = strstr(buf, name);
    if (!p) return;
    p = strchr(p, '"');
    if (!p) return;
    q = strchr(p+1, '"');
    if (!q) return;
    if (len > q-p) len = (int)(q-p);
    strlcpy(dest, p+1, len);
}

int copy_stream(FILE* in, FILE* out) {
    char buf[1024];
    int n, m;
    while (1) {
        n = (int)fread(buf, 1, 1024, in);
        m = (int)fwrite(buf, 1, n, out);
        if (m != n) return ERR_FWRITE;
        if (n < 1024) break;
    }
    return 0;
}

// append to a malloc'd string
//
int strcatdup(char*& p, char* buf) {
    p = (char*)realloc(p, strlen(p) + strlen(buf)+1);
    if (!p) {
        return ERR_MALLOC;
    }
    strcat(p, buf);
    return 0;
}

// Copy from a file to a malloc'd string until the end tag is reached
// Does NOT copy the start and end tags.
//
int dup_element_contents(FILE* in, const char* end_tag, char** pp) {
    char line[256];
    int bufsize = 4000000;
    int nused=0;        // not counting ending NULL
    char* buf = (char*)malloc(bufsize);

    // Start with a big buffer.
    // When done, copy to an exact-size buffer
    //
    while (fgets(line, 256, in)) {
        if (strstr(line, end_tag)) {
            *pp = (char*)malloc(nused+1);
            strcpy(*pp, buf);
            free(buf);
            return 0;
        }
        int n = (int)strlen(line);
        if (nused + n >= bufsize) {
            bufsize *= 2;
            buf = (char*)realloc(buf, bufsize);
        }
        strcpy(buf+nused, line);
        nused += n;
    }
    free(buf);
    return ERR_XML_PARSE;
}

int dup_element(FILE* in, const char* tag_name, char** pp) {
    char buf[256], end_tag[256];
    int retval;

    sprintf(buf, "<%s>\n", tag_name);
    sprintf(end_tag, "</%s>", tag_name);

    char* p = strdup(buf);
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            sprintf(buf, "</%s>\n", tag_name);
            retval = strcatdup(p, buf);
            if (retval) return retval;
            *pp = p;
            return 0;
        }
        retval = strcatdup(p, buf);
        if (retval) return retval;
    }
    return ERR_XML_PARSE;
}

// copy from a file to static buffer
//
int copy_element_contents(FILE* in, const char* end_tag, char* p, int len) {
    char buf[256];
    int n;

    strcpy(p, "");
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        n = (int)strlen(buf);
        if (n >= len-1) return ERR_XML_PARSE;
        strcat(p, buf);
        len -= n;
    }
    return ERR_XML_PARSE;
}

int copy_element_contents(FILE* in, const char* end_tag, string& str) {
    char buf[256];

    str = "";
    while (fgets(buf, 256, in)) {
        if (strstr(buf, end_tag)) {
            return 0;
        }
        str += buf;
    }
    return ERR_XML_PARSE;
}

// replace XML element contents (element must be present)
//
void replace_element_contents(
    char* buf, const char* start, const char* end, const char* replacement
) {
    char temp[4096], *p, *q;

    p = strstr(buf, start);
    p += strlen(start);
    q = strstr(p, end);
    strlcpy(temp, q, sizeof(temp));
    strcpy(p, replacement);
    strcat(p, temp);
}

// if the string contains a substring of the form X...Y,
// remove the first such.
bool remove_element(char* buf, const char* start, const char* end) {
    char* p, *q;
    p = strstr(buf, start);
    if (!p) return false;
    q = strstr(p+strlen(start), end);
    if (!q) return false;
    strcpy_overlap(p, q+strlen(end));
    return true;
}

// replace a substring.  Do at most one instance.
//
bool str_replace(char* str, const char* substr, const char* replacement) {
    char temp[4096], *p;

    p = strstr(str, substr);
    if (!p) return false;
    int n = (int)strlen(substr);
    strcpy(temp, p+n);
    strcpy(p, replacement);
    strcat(p, temp);
    return true;
}

// if the given XML has an element of the form
// <venue name="venue_name">
//   ...
// </venue>
// then return the contents of that element.
// Otherwise strip out all <venue> elements
//
void extract_venue(const char* in, const char* venue_name, char* out) {
    const char* p, *q;
    char* wp;
    char buf[256];
    sprintf(buf, "<venue name=\"%s\">", venue_name);
    p = strstr(in, buf);
    if (p) {
        // prefs contain the specified venue
        //
        p += strlen(buf);
        strcpy(out, p);
        wp = strstr(out, "</venue");
        if (wp) *wp = 0;
    } else {
        // prefs don't contain the specified venue
        //
        q = in;
        strcpy(out, "");
           while (1) {
               p = strstr(q, "<venue");
               if (!p) {
                   strcat(out, q);
                break;
            }
               strncat(out, q, p-q);
               q = strstr(p, "</venue>");
               if (!q) break;
               q += strlen("</venue>");
           }
    }
}

// copy a line from the given string.
// kinda like fgets() when you're reading from a string
//
char* sgets(char* buf, int len, char*& in) {
    char* p;

    p = strstr(in, "\n");
    if (!p) return NULL;
    *p = 0;
    strlcpy(buf, in, len);
    *p = '\n';
    in = p+1;
    return buf;
}

void non_ascii_escape(const char* in, char* out, int len) {
    char buf[256], *p;

    p = out;

    for (; *in; in++) {
        int x = (int) *in;
        x &= 0xff;   // just in case
        if (x>127) {
            sprintf(buf, "&#%d;", x);
            strcpy(p, buf);
            p += strlen(buf);
        } else {
            *p++ = x;
        }
        if (p > out + len - 8) break;
    }
    *p = 0;
}

// NOTE: these used to take std::string instead of char* args.
// But this performed poorly.
//
// NOTE: output buffer should be 6X size of input
//
void xml_escape(const char* in, char* out, int len) {
    char buf[256], *p;

    p = out;

    for (; *in; in++) {
        int x = (int) *in;
        x &= 0xff;   // just in case
        if (x == '<') {
            strcpy(p, "&lt;");
            p += 4;
        } else if (x == '&') {
            strcpy(p, "&amp;");
            p += 5;
        } else if (x>127) {
            sprintf(buf, "&#%d;", x);
            strcpy(p, buf);
            p += strlen(buf);
        } else if (x<32) {
            switch(x) {
            case 9:
            case 10:
            case 13:
                sprintf(buf, "&#%d;", x);
                strcpy(p, buf);
                p += strlen(buf);
                break;
            }
        } else {
            *p++ = x;
        }
        if (p > out + len - 8) break;
    }
    *p = 0;
}

// Note: XML unescaping never increases string length
//
void xml_unescape(string& in) {
    int n = (int)in.size()+1+16;      // +16 avoids valgrind warnings
    char* buf = (char*)malloc(n);
    strcpy(buf, in.c_str());
    xml_unescape(buf);
    in = buf;
    free(buf);
}

void xml_unescape(char* buf) {
    char* out = buf;
    char* in = buf;
    char* p;
    while (*in) {
        if (*in != '&') {       // avoid strncmp's if possible
            *out++ = *in++;
        } else if (!strncmp(in, "&lt;", 4)) {
            *out++ = '<';
            in += 4;
        } else if (!strncmp(in, "&gt;", 4)) {
            *out++ = '>';
            in += 4;
        } else if (!strncmp(in, "&quot;", 4)) {
            *out++ = '"';
            in += 6;
        } else if (!strncmp(in, "&apos;", 4)) {
            *out++ = '\'';
            in += 6;
        } else if (!strncmp(in, "&amp;", 5)) {
            *out++ = '&';
            in += 5;
        } else if (!strncmp(in, "&#", 2)) {
            in += 2;
            char c = atoi(in);
            *out++ = c;
            p = strchr(in, ';');
            if (p) {
                in = p+1; 
            } else {
                while (isdigit(*in)) in++;
            }
        } else {
            *out++ = *in++;
        }
    }
    *out = 0;
}

// we got an unrecognized line.
// If it has two <'s (e.g. <foo>xx</foo>) return 0.
// If it's of the form <foo/> return 0.
// If it's of the form <foo> then scan for </foo> and return 0.
// Otherwise return ERR_XML_PARSE
//
int skip_unrecognized(char* buf, MIOFILE& fin) {
    char* p, *q, buf2[256];
    std::string close_tag;

    p = strchr(buf, '<');
    if (!p) {
        return ERR_XML_PARSE;
    }
    if (strchr(p+1, '<')) {
        return 0;
    }
    q = strchr(p+1, '>');
    if (!q) {
        return ERR_XML_PARSE;
    }
    if (q[-1] == '/') return 0;
    *q = 0;
    close_tag = string("</") + string(p+1) + string(">");
    while (fin.fgets(buf2, 256)) {
        if (strstr(buf2, close_tag.c_str())) {
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

XML_PARSER::XML_PARSER(MIOFILE* _f) {
    f = _f;
}

// read until find non-whitespace char.
// Return the char in the reference param
// Return true iff reached EOF
//
bool XML_PARSER::scan_nonws(int& first_char) {
    char c;
    while (1) {
        c = f->_getc();
        if (c == EOF) return true;
        unsigned char uc = c;
        if (isspace(uc)) continue;
        first_char = c;
        return false;
    }
}

#define XML_PARSE_COMMENT   1
#define XML_PARSE_EOF       2
#define XML_PARSE_CDATA     3
#define XML_PARSE_TAG       4
#define XML_PARSE_DATA      5

int XML_PARSER::scan_comment() {
    char buf[256];
    char* p = buf;
    while (1) {
        int c = f->_getc();
        if (c == EOF) return XML_PARSE_EOF;
        *p++ = c;
        *p = 0;
        if (strstr(buf, "-->")) {
            return XML_PARSE_COMMENT;
        }
        if (strlen(buf) > 32) {
            strcpy_overlap(buf, buf+16);
            p -= 16;
        }
    }
}

int XML_PARSER::scan_cdata(char* buf, int len) {
    char* p = buf;
    len--;
    while (1) {
        int c = f->_getc();
        if (c == EOF) return XML_PARSE_EOF;
        if (len) {
            *p++ = c;
            len--;
        }
        if (c == '>') {
            *p = 0;
            char* q = strstr(buf, "]]>");
            if (q) {
                *q = 0;
                return XML_PARSE_CDATA;
            }
        }
    }
}

// we just read a <; read until we find a >.
// Given <tag [attr=val attr=val] [/]>:
// - copy tag (or tag/) to buf
// - copy "attr=val attr=val" to attr_buf
//
// Return either
// XML_PARSE_TAG
// XML_PARSE_COMMENT
// XML_PARSE_EOF
// XML_PARSE_CDATA
//
int XML_PARSER::scan_tag(
    char* buf, int _tag_len, char* attr_buf, int attr_len
) {
    int c;
    char* buf_start = buf;
    bool found_space = false;
    int tag_len = _tag_len;

    for (int i=0; ; i++) {
        c = f->_getc();
        if (c == EOF) return XML_PARSE_EOF;
        if (c == '>') {
            *buf = 0;
            if (attr_buf) *attr_buf = 0;
            return XML_PARSE_TAG;
        }
        if (isspace(c)) {
            if (found_space && attr_buf) {
                if (--attr_len > 0) {
                    *attr_buf++ = c;
                }
            }
            found_space = true;
        } else if (c == '/') {
            if (--tag_len > 0) {
                *buf++ = c;
            }
        } else {
            if (found_space) {
                if (attr_buf) {
                    if (--attr_len > 0) {
                        *attr_buf++ = c;
                    }
                }
            } else {
                if (--tag_len > 0) {
                    *buf++ = c;
                }
            }
        }

        // check for comment start
        //
        if (i==2 && !strncmp(buf_start, "!--", 3)) {
            return scan_comment();
        }
        if (i==7 && !strncmp(buf_start, "![CDATA[", 8)) {
            return scan_cdata(buf_start, tag_len);
        }
    }
}

// read and copy text to buf; stop when find a <;
// ungetc() that so we read it again
// Return true iff reached EOF
//
bool XML_PARSER::copy_until_tag(char* buf, int len) {
    int c;
    while (1) {
        c = f->_getc();
        if (c == EOF) return true;
        if (c == '<') {
            f->_ungetc(c);
            *buf = 0;
            return false;
        }
        if (--len > 0) {
            *buf++ = c;
        }
    }
}

// Scan something, either tag or text.
// Strip whitespace at start and end.
// Return true iff reached EOF
//
int XML_PARSER::get_aux(char* buf, int len, char* attr_buf, int attr_len) {
    bool eof;
    int c, retval;

    while (1) {
        eof = scan_nonws(c);
        if (eof) return XML_PARSE_EOF;
        if (c == '<') {
            retval = scan_tag(buf, len, attr_buf, attr_len);
            if (retval == XML_PARSE_EOF) return retval;
            if (retval == XML_PARSE_COMMENT) continue;
        } else {
            buf[0] = c;
            eof = copy_until_tag(buf+1, len-1);
            if (eof) return XML_PARSE_EOF;
            retval = XML_PARSE_DATA;
        }
        strip_whitespace(buf);
        return retval;
    }
}

bool XML_PARSER::get(
    char* buf, int len, bool& _is_tag, char* attr_buf, int attr_len
) {
    switch (get_aux(buf, len, attr_buf, attr_len)) {
    case XML_PARSE_EOF: return true;
    case XML_PARSE_TAG:
        _is_tag = true;
        break;
    case XML_PARSE_DATA:
    case XML_PARSE_CDATA:
    default:
        _is_tag = false;
        break;
    }
    return false;
}

#define MAX_XML_STRING  262144

// We just parsed "parsed_tag".
// If it matches "start_tag", and is followed by a string
// and by the matching close tag, return the string in "buf",
// and return true.
//
bool XML_PARSER::parse_str(const char* start_tag, char* buf, int len) {
    bool eof;
    char end_tag[256], tag[256];

    // handle the archaic form <tag/>, which means empty string
    //
    size_t n = strlen(parsed_tag);
    if (parsed_tag[n-1] == '/') {
        strcpy(tag, parsed_tag);
        tag[n-1] = 0;
        if (!strcmp(tag, start_tag)) {
            strcpy(buf, "");
            return true;
        }
    }

    // check for start tag
    //
    if (strcmp(parsed_tag, start_tag)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    // get text after start tag
    //
    int retval = get_aux(buf, len, 0, 0);
    if (retval == XML_PARSE_EOF) return false;

    // if it's the end tag, return empty string
    //
    if (retval == XML_PARSE_TAG) {
        if (strcmp(buf, end_tag)) {
            return false;
        } else {
            strcpy(buf, "");
            return true;
        }
    }

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    if (retval != XML_PARSE_CDATA) {
        xml_unescape(buf);
    }
    return true;
}

bool XML_PARSER::parse_string(const char* start_tag, string& str) {
    char buf[MAX_XML_STRING];
    bool flag = parse_str(start_tag, buf, sizeof(buf));
    if (!flag) return false;
    str = buf;
    return true;
}

// Same, for integers
//
bool XML_PARSER::parse_int(const char* start_tag, int& i) {
    char buf[256], *end;
    bool eof;
    char end_tag[256], tag[256];

    if (strcmp(parsed_tag, start_tag)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    eof = get(buf, sizeof(buf), is_tag);
    if (eof) return false;
    if (is_tag) {
        if (!strcmp(buf, end_tag)) {
            i = 0;      // treat <foo></foo> as <foo>0</foo>
            return true;
        } else {
            return false;
        }
    }
    errno = 0;
    int val = strtol(buf, &end, 0);
    if (errno) return false;
    if (end != buf+strlen(buf)) return false;

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    i = val;
    return true;
}

// Same, for doubles
//
bool XML_PARSER::parse_double(const char* start_tag, double& x) {
    char buf[256], *end;
    bool eof;
    char end_tag[256], tag[256];

    if (strcmp(parsed_tag, start_tag)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    eof = get(buf, sizeof(buf), is_tag);
    if (eof) return false;
    if (is_tag) {
        if (!strcmp(buf, end_tag)) {
            x = 0;      // treat <foo></foo> as <foo>0</foo>
            return true;
        } else {
            return false;
        }
    }
    errno = 0;
    double val = strtod(buf, &end);
    if (errno) return false;
    if (end != buf+strlen(buf)) return false;

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    x = val;
    return true;
}

// Same, for unsigned long
//
bool XML_PARSER::parse_ulong(const char* start_tag, unsigned long& x) {
    char buf[256], *end;
    bool eof;
    char end_tag[256], tag[256];

    if (strcmp(parsed_tag, start_tag)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    eof = get(buf, sizeof(buf), is_tag);
    if (eof) return false;
    if (is_tag) {
        if (!strcmp(buf, end_tag)) {
            x = 0;      // treat <foo></foo> as <foo>0</foo>
            return true;
        } else {
            return false;
        }
    }
    errno = 0;
    unsigned long val = strtoul(buf, &end, 0);
    if (errno) return false;
    if (end != buf+strlen(buf)) return false;

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    x = val;
    return true;
}

// Same, for unsigned long long
//
bool XML_PARSER::parse_ulonglong(const char* start_tag, unsigned long long& x) {
    char buf[256], *end=0;
    bool eof;
    char end_tag[256], tag[256];

    if (strcmp(parsed_tag, start_tag)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    eof = get(buf, sizeof(buf), is_tag);
    if (eof) return false;
    if (is_tag) {
        if (!strcmp(buf, end_tag)) {
            x = 0;      // treat <foo></foo> as <foo>0</foo>
            return true;
        } else {
            return false;
        }
    }
    errno = 0;
    unsigned long long val = boinc_strtoull(buf, &end, 0);
    if (errno) return false;
    if (end != buf+strlen(buf)) return false;

    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    x = val;
    return true;
}

// Same, for bools
//
bool XML_PARSER::parse_bool(const char* start_tag, bool& b) {
    char buf[256], *end;
    bool eof;
    char end_tag[256], tag[256];

    // handle the archaic form <tag/>, which means true
    //
    strcpy(tag, start_tag);
    strcat(tag, "/");
    if (!strcmp(parsed_tag, tag)) {
        b = true;
        return true;
    }

    // otherwise look for something of the form <tag>int</tag>
    //
    if (strcmp(parsed_tag, start_tag)) return false;

    eof = get(buf, sizeof(buf), is_tag);
    if (eof) return false;
    if (is_tag) return false;
    bool val = (strtol(buf, &end, 0) != 0);
    if (end != buf+strlen(buf)) return false;

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);
    eof = get(tag, sizeof(tag), is_tag);
    if (eof) return false;
    if (!is_tag) return false;
    if (strcmp(tag, end_tag)) return false;
    b = val;
    return true;
}

// parse a start tag (optionally preceded by <?xml>)
//
bool XML_PARSER::parse_start(const char* start_tag) {
    char tag[256];
    bool eof;

    eof = get(tag, sizeof(tag), is_tag);
    if (eof || !is_tag ) {
        return false;
    }
    if (strstr(tag, "?xml")) {
        eof = get(tag, sizeof(tag), is_tag);
        if (eof || !is_tag ) {
            return false;
        }
    }
    if (strcmp(tag, start_tag)) {
        return false;
    }
    return true;
}

// copy everything up to (but not including) the given end tag.
// The copied text may include XML tags.
// strips whitespace.
//
int XML_PARSER::element_contents(const char* end_tag, char* buf, int buflen) {
    int n=0;
    int retval=0;
    while (1) {
        if (n == buflen-1) {
            retval = ERR_XML_PARSE;
            break;
        }
        int c = f->_getc();
        if (c == EOF) {
            retval = ERR_XML_PARSE;
            break;
        }
        buf[n++] = c;
        buf[n] = 0;
        char* p = strstr(buf, end_tag);
        if (p) {
            *p = 0;
            break;
        }
    }
    buf[n] = 0;
    strip_whitespace(buf);
    return retval;
}

#if 0
int XML_PARSER::element_contents(const char* end_tag, string& buf) {
    int retval=0;
    while (1) {
        int c = f->_getc();
        if (c == EOF) {
            retval = ERR_XML_PARSE;
            break;
        }
        buf += c;
        char* p = strstr(buf.c_str(), end_tag);
        if (p) {
            int k = strlen(end_tag);
            int n = buf.length();
            buf.erase(n-k, k);
            break;
        }
    }
    strip_whitespace(buf);
    return retval;
}
#endif

// We got an unexpected tag.
// If it's an end tag, do nothing.
// Otherwise skip until the end tag, if any
//
void XML_PARSER::skip_unexpected(
    const char* start_tag, bool verbose, const char* where
) {
    char tag[256], end_tag[256];

    if (verbose) {
        fprintf(stderr, "Unrecognized XML in %s: %s\n", where, start_tag);
    }
    if (strchr(start_tag, '/')) return;
    sprintf(end_tag, "/%s", start_tag);
    while (!get(tag, sizeof(tag), is_tag)) {
        if (verbose) {
            fprintf(stderr, "Skipping: %s\n", tag);
        }
        if (!is_tag) continue;
        if (!strcmp(tag, end_tag)) return;
        skip_unexpected(tag, false, where);
    }
}

// we just parsed a tag.
// copy this entire element, including start and end tags, to the buffer
//
int XML_PARSER::copy_element(string& out) {
    char end_tag[256], buf[1024];

    // handle <foo/> case
    //
    size_t n = strlen(parsed_tag);
    if (parsed_tag[n-1] == '/') {
        out = "<";
        out += parsed_tag;
        out += ">";
        return 0;
    }
    if (strchr(parsed_tag, '/')) return ERR_XML_PARSE;
    out = "<";
    out += parsed_tag;
    out += ">";
    sprintf(end_tag, "</%s>", parsed_tag);
    int retval = element_contents(end_tag, buf, sizeof(buf));
    if (retval) return retval;
    out += buf;
    out += end_tag;
    return 0;
}
