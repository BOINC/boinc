// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2023 University of California
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

// This file contains two XML parsers:
//
// 1) a very crude one, which assumes all elements are either single-line or
// have start and end tags on separate lines.
// This is meant to be used ONLY for parsing XML files produced
// by the BOINC scheduling server or client.
//
// 2) a better one (class XML_PARSER) which parses arbitrary XML

#if defined(_WIN32)
#include "boinc_win.h"
#else
#include "config.h"
#include <cstring>
#include <cstdlib>
#include <string>
#include <cmath>
#include <ctype.h>
#include <errno.h>
#if HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#endif

#ifdef __APPLE__
#include <xlocale.h>
#endif

#include "boinc_stdio.h"

#include "error_numbers.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "parse.h"

using std::string;

unsigned long long boinc_strtoull(const char *str, char **endptr, int base) {
#if (defined (__cplusplus) && __cplusplus > 199711L) || defined(HAVE_STRTOULL) || defined(__MINGW32__)
    return strtoull(str, endptr, base);
#elif defined(_WIN32) && !defined(__MINGW32__)
    return _strtoui64(str, endptr, base);
#else
    char buf[64];
    char *p;
    unsigned long long y;
    strncpy(buf, str, sizeof(buf)-1);
    strip_whitespace(buf);
    p = strstr(buf, "0x");
    if (!p) p = strstr(buf, "0X");
    if (p) {
        sscanf(p, "%llx", &y);
    } else {
        sscanf(buf, "%llu", &y);
    }
    return y;
#endif
}

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
    snprintf(tag2, sizeof(tag2), "<%s/>", tag);
    snprintf(tag3, sizeof(tag3), "<%s />", tag);
    if (match_tag(buf, tag2) || match_tag(buf, tag3)) {
        result = true;
        return true;
    }
    snprintf(tag2, sizeof(tag2), "<%s>", tag);
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
    if (!p) return false;
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
    size_t n, m;
    while (1) {
        n = boinc::fread(buf, 1, 1024, in);
        m = boinc::fwrite(buf, 1, n, out);
        if (m != n) return ERR_FWRITE;
        if (n < 1024) break;
    }
    return 0;
}

// append to a malloc'd string
// If reallocation fails, the pointer p remains unchanged, and the data will
// not be freed. (strong exception safety)
//
int strcatdup(char*& p, char* buf) {
    char* new_p = (char*)realloc(p, strlen(p) + strlen(buf)+1);
    if (!new_p) {
        return ERR_MALLOC;
    }
    p = new_p;
    strcat(p, buf);
    return 0;
}

// Copy from a file to a malloc'd string until the end tag is reached
// Does NOT copy the start and end tags.
//
int dup_element_contents(FILE* in, const char* end_tag, char** pp) {
    string buf;
    int retval = copy_element_contents(in, end_tag, buf);
    if (retval) return retval;
    *pp = strdup(buf.c_str());
    return 0;
}

int dup_element(FILE* in, const char* tag_name, char** pp) {
    char start_tag[256], end_tag[256];
    string buf, buf2;

    snprintf(start_tag, sizeof(start_tag), "<%s>\n", tag_name);
    snprintf(end_tag, sizeof(end_tag), "</%s>", tag_name);

    int retval = copy_element_contents(in, end_tag, buf);
    if (retval) return retval;
    buf2 = start_tag + buf + end_tag;
    *pp = strdup(buf2.c_str());
    return 0;
}

// copy input up to but not including end tag, to a char array
//
int copy_element_contents(FILE* in, const char* end_tag, char* p, size_t len) {
    string buf;
    int retval = copy_element_contents(in, end_tag, buf);
    if (retval) return retval;
    if (buf.size() > len-1) {
        return ERR_BUFFER_OVERFLOW;
    }
    strlcpy(p, buf.c_str(), len);
    return 0;
}

// copy input up to but not including end tag, to a string
//
int copy_element_contents(FILE* in, const char* end_tag, string& str) {
    int c;
    size_t end_tag_len = strlen(end_tag);
    size_t n = 0;

    str = "";
    while (1) {
        c = boinc::fgetc(in);
        if (c == EOF) break;
        str += c;
        n++;
        if (n < end_tag_len) {
            continue;
        }
        const char* p = str.c_str() + n - end_tag_len;
        if (!strcmp(p, end_tag)) {
            str.erase(n-end_tag_len, end_tag_len);
            return 0;
        }
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
    safe_strcpy(temp, p+n);
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
void extract_venue(const char* in, const char* venue_name, char* out, int len) {
    const char* p, *q;
    char* wp;
    char buf[256];
    const size_t venue_close_tag_len = strlen("</venue>");
    snprintf(buf, sizeof(buf), "<venue name=\"%s\">", venue_name);
    p = strstr(in, buf);
    if (p) {
        // prefs contain the specified venue
        //
        p += strlen(buf);
        strlcpy(out, p, len);
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
                   strlcat(out, q, len);
                   break;
               }
               strncat(out, q, p-q);
               q = strstr(p, "</venue>");
               if (!q) break;
               q += venue_close_tag_len;
           }
    }
}

// copy a line from the given string.
// kinda like fgets() when you're reading from a string
//
char* sgets(char* buf, int len, char*& in) {
    char* p;

    p = strchr(in, '\n');
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
            snprintf(buf, sizeof(buf), "&#%d;", x);
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
            snprintf(buf, sizeof(buf), "&#%d;", x);
            strcpy(p, buf);
            p += strlen(buf);
        } else if (x<32) {
            switch(x) {
            case 9:
            case 10:
            case 13:
                snprintf(buf, sizeof(buf), "&#%d;", x);
                strcpy(p, buf);
                p += strlen(buf);
                break;
            }
        } else if (x == ']') {
            // two stage check, strncmp() is slow
            if (!strncmp(in, "]]>", 3)) {
                strcpy(p, "]]&gt;");
                p += 6;
                in += 2;  // +1 from for loop
            } else {
                *p++ = x;
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
    bool goodescape;
    while (*in) {
        goodescape = false;
        if (*in != '&') {       // avoid strncmp's if possible
            *out++ = *in++;
        } else if (!strncmp(in, "&lt;", 4)) {
            *out++ = '<';
            in += 4;
        } else if (!strncmp(in, "&gt;", 4)) {
            *out++ = '>';
            in += 4;
        } else if (!strncmp(in, "&quot;", 6)) {
            *out++ = '"';
            in += 6;
        } else if (!strncmp(in, "&apos;", 6)) {
            *out++ = '\'';
            in += 6;
        } else if (!strncmp(in, "&amp;", 5)) {
            *out++ = '&';
            in += 5;
        } else if (!strncmp(in, "&#xD;", 5) || !strncmp(in, "&#xd;", 5)) {
            *out++ = '\r';
            in += 5;
        } else if (!strncmp(in, "&#xA;", 5) || !strncmp(in, "&#xa;", 5)) {
            *out++ = '\n';
            in += 5;
        } else if (!strncmp(in, "&#", 2)) {
            //If escape is poorly formed or outside of char size, then print as is.
            in += 2;
            p = strchr(in, ';');
            if (!p || *in == ';') { //No end semicolon found or it was formatted as &#;
                *out++ = '&';
                *out++ = '#';
            } else {
                //Check that escape is formed correctly
                for (unsigned int i = 0; i < 4 || i < strlen(in); i++) {
                    if (!isdigit(*(in + i)) && *(in + i) != ';') {
                        //Found something other than a single digit.
                        break;
                    }
                    if (*(in + i) == ';') {
                        goodescape = true;
                        break;
                    }
                }
                int ascii = atoi(in);

                if (goodescape && ascii < 256) {
                    *out++ = ascii;
                    in = p + 1;
                } else {
                    *out++ = '&';
                    *out++ = '#';
                }
            }
        } else {
            *out++ = *in++;
        }
    }
    *out = 0;
}

#if 0
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
#endif

XML_PARSER::XML_PARSER(MIOFILE* _f) {
    strcpy(parsed_tag, "");
    is_tag = false;
    f = _f;
}

int XML_PARSER::scan_comment() {
    char buf[256];
    char* p = buf;
    while (1) {
        int c = f->_getc();
        if (!c || c == EOF) return XML_PARSE_EOF;
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
        if (!c || c == EOF) return XML_PARSE_EOF;
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

static inline bool is_empty_string(char* parsed_tag, const char* start_tag) {
    size_t n = strlen(parsed_tag);
    char tag[TAG_BUF_LEN];

    // handle the archaic form <tag/>, which means empty string
    //
    if (parsed_tag[n-1] == '/') {
        strcpy(tag, parsed_tag);
        tag[n-1] = 0;
        if (!strcmp(tag, start_tag)) {
            return true;
        }
    }
    return false;
}

// we've parsed the start tag of a string; parse the string itself.
//
bool XML_PARSER::parse_str_aux(const char* start_tag, char* buf, int len) {
    bool eof;
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

    end_tag[0] = '/';
    strcpy(end_tag+1, start_tag);

    // get text after start tag
    //
    int retval = get_aux(buf, len, 0, 0);
    if (retval == XML_PARSE_EOF) return false;
    if (retval == XML_PARSE_OVERFLOW) return false;

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

// We just parsed "parsed_tag".
// If it matches "start_tag", and is followed by a string
// and by the matching close tag, return the string in "buf",
// and return true.
//
bool XML_PARSER::parse_str(const char* start_tag, char* buf, int len) {
    if (is_empty_string(parsed_tag, start_tag)) {
        strcpy(buf, "");
        return true;
    }
    if (strcmp(parsed_tag, start_tag)) return false;
    return parse_str_aux(start_tag, buf, len);
}

#define MAX_XML_STRING  262144

// same, for std::string
//
bool XML_PARSER::parse_string(const char* start_tag, string& str) {
    bool flag = false;
    if (is_empty_string(parsed_tag, start_tag)) {
        str = "";
        return true;
    }
    if (strcmp(parsed_tag, start_tag)) return false;
    char *buf=(char *)malloc(MAX_XML_STRING);
    if (buf) {
        flag = parse_str_aux(start_tag, buf, MAX_XML_STRING);
        if (flag) {
            str = buf;
        }
        free(buf);
    }
    return flag;
}

// Same, for integers
//
bool XML_PARSER::parse_int(const char* start_tag, int& i) {
    char buf[256], *end;
    bool eof;
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

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

// Same, for long
//
bool XML_PARSER::parse_long(const char* start_tag, long& i) {
    char buf[256], *end;
    bool eof;
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

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
    long val = strtol(buf, &end, 0);
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
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

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
#if (defined(__APPLE__) && defined(BUILDING_MANAGER))
// MacOS 13.3.1 apparently broke per-thread locale uselocale()
    double val = strtod_l(buf, &end, LC_C_LOCALE);
#else
    double val = strtod(buf, &end);
#endif
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
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

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
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

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
    char end_tag[TAG_BUF_LEN], tag[TAG_BUF_LEN];

    // handle the archaic form <tag/>, which means true
    //
    safe_strcpy(tag, start_tag);
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
    char tag[TAG_BUF_LEN];
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

// We got an unexpected tag.
// If it's an end tag, do nothing.
// Otherwise skip until the end tag, if any
//
void XML_PARSER::skip_unexpected(
    const char* start_tag, bool verbose, const char* where
) {
    char buf[TAG_BUF_LEN], end_tag[TAG_BUF_LEN];

    if (verbose) {
        boinc::fprintf(stderr,
            "%s: Unrecognized XML tag '<%s>' in %s; skipping\n",
            time_to_string(dtime()), start_tag, where
        );
    }
    if (strchr(start_tag, '/')) return;
    snprintf(end_tag, sizeof(end_tag), "/%s", start_tag);

    while (1) {
        int c;
        bool eof = scan_nonws(c);
        if (eof) return;
        if (c == '<') {
            int retval = scan_tag(buf, sizeof(buf), 0, 0);
            if (retval != XML_PARSE_TAG) continue;
            if (!strcmp(buf, end_tag)) return;
            skip_unexpected(buf, false, where);
        }
    }
}

// we just parsed a tag.
// copy this entire element, including start and end tags, to the buffer
//
int XML_PARSER::copy_element(string& out) {
    char end_tag[TAG_BUF_LEN], buf[ELEMENT_BUF_LEN];

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
    snprintf(end_tag, sizeof(end_tag), "</%.256s>", parsed_tag);
    int retval = element_contents(end_tag, buf, sizeof(buf));
    if (retval) return retval;
    out += buf;
    out += end_tag;
    return 0;
}
