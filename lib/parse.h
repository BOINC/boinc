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

#ifndef BOINC_PARSE_H
#define BOINC_PARSE_H

#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef __APPLE__
#include <xlocale.h>
#endif

#include "miofile.h"
#include "error_numbers.h"
#include "str_util.h"

extern bool boinc_is_finite(double);
    // avoid including util.h (kludge)

// see parse_test.cpp for example usage of XML_PARSER

#define XML_PARSE_COMMENT   1
#define XML_PARSE_EOF       2
#define XML_PARSE_CDATA     3
#define XML_PARSE_TAG       4
#define XML_PARSE_DATA      5
#define XML_PARSE_OVERFLOW  6

#define TAG_BUF_LEN         4096
    // max tag length
#define ELEMENT_BUF_LEN     65536
    // max element length (matches BLOB_SIZE, max size of XML fields in DB)

struct XML_PARSER {
    int scan_comment();
    int scan_cdata(char*, int);
    char parsed_tag[TAG_BUF_LEN];
    bool is_tag;
    MIOFILE* f;
    XML_PARSER(MIOFILE*);
    void init(MIOFILE* mf) {
        f = mf;
    }
    // read and copy text to buf; stop when find a <;
    // ungetc() that so we read it again
    // Return XML_PARSE_DATA if successful
    //
    inline int copy_until_tag(char* buf, int len) {
        int c;
        while (1) {
            c = f->_getc();
            if (!c || c == EOF) return XML_PARSE_EOF;
            if (c == '<') {
                f->_ungetc(c);
                *buf = 0;
                return XML_PARSE_DATA;
            }
            if (--len <= 0) {
                return XML_PARSE_OVERFLOW;
            }
            *buf++ = (char)c;
        }
    }

    // return true if EOF or error
    //
    inline bool get(
        char* buf, int len, bool& _is_tag, char* attr_buf=0, int attr_len=0
    ) {
        switch (get_aux(buf, len, attr_buf, attr_len)) {
        case XML_PARSE_EOF:
        case XML_PARSE_OVERFLOW:
            return true;
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

    inline bool get_tag(char* ab=0, int al=0) {
        if (get(parsed_tag, sizeof(parsed_tag), is_tag, ab, al)) {
            return true;
        }
        if (strlen(parsed_tag) > TAG_BUF_LEN-10) {
            parsed_tag[TAG_BUF_LEN-10] = 0;
        }
        return false;
    }
    inline bool match_tag(const char* tag) {
        return !strcmp(parsed_tag, tag);
    }

    // read until find non-whitespace char.
    // Return the char in the reference param
    // Return true iff reached EOF
    //
    inline bool scan_nonws(int& first_char) {
        int c;
        while (1) {
            c = f->_getc();
            if (!c || c == EOF) return true;
            if (isascii(c) && isspace(c)) continue;
            first_char = c;
            return false;
        }
    }

    // Scan something, either tag or text.
    // Strip whitespace at start and end
    // (however, the supplied buffer must accommodate this white space).
    // Ignore comments.
    // Return true iff reached EOF
    //
    inline int get_aux(
        char* buf, int len, char* attr_buf, int attr_len
    ) {
        bool eof;
        int c, retval;

        while (1) {
            eof = scan_nonws(c);
            if (eof) return XML_PARSE_EOF;
            if (c == '<') {
                retval = scan_tag(buf, len, attr_buf, attr_len);
                if (retval == XML_PARSE_EOF) return retval;
                if (retval == XML_PARSE_OVERFLOW) return retval;
                if (retval == XML_PARSE_COMMENT) continue;
            } else {
                buf[0] = (char)c;
                retval = copy_until_tag(buf+1, len-1);
                if (retval != XML_PARSE_DATA) return retval;
            }
            strip_whitespace(buf);
            return retval;
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
    inline int scan_tag(
        char* buf, int _tag_len, char* attr_buf=0, int attr_len=0
    ) {
        int c;
        char* buf_start = buf;
        bool found_space = false;
        int tag_len = _tag_len;

        for (int i=0; ; i++) {
            c = f->_getc();
            if (!c || c == EOF) return XML_PARSE_EOF;
            if (c == '>') {
                *buf = 0;
                if (attr_buf) *attr_buf = 0;
                return XML_PARSE_TAG;
            }
            if (isascii(c) && isspace(c)) {
                if (found_space && attr_buf) {
                    if (--attr_len > 0) {
                        *attr_buf++ = (char)c;
                    }
                }
                found_space = true;
            } else if (c == '/') {
                if (--tag_len > 0) {
                    *buf++ = (char)c;
                } else {
                    return XML_PARSE_OVERFLOW;
                }
            } else {
                if (found_space) {
                    if (attr_buf) {
                        if (--attr_len > 0) {
                            *attr_buf++ = (char)c;
                        }
                    }
                } else {
                    if (--tag_len > 0) {
                        *buf++ = (char)c;
                    } else {
                        return XML_PARSE_OVERFLOW;
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

    // copy everything up to (but not including) the given end tag.
    // The copied text may include XML tags.
    // strips start/end whitespace.
    //
    inline int element_contents(const char* end_tag, char* buf, int buflen) {
        int n=0;
        int retval=0;
        while (1) {
            if (n == buflen-1) {
                retval = ERR_XML_PARSE;
                break;
            }
            int c = f->_getc();
            if (!c || c == EOF) {
                retval = ERR_XML_PARSE;
                break;
            }
            buf[n++] = (char)c;
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
    bool parse_str_aux(const char*, char*, int);

    // interface starts here
    //
    bool parse_start(const char*);
    bool parse_str(const char*, char*, int);
    bool parse_string(const char*, std::string&);
    bool parse_int(const char*, int&);
    bool parse_long(const char*, long&);
    bool parse_double(const char*, double&);
    bool parse_ulong(const char*, unsigned long&);
    bool parse_ulonglong(const char*, unsigned long long&);
    bool parse_bool(const char*, bool&);
    int copy_element(std::string&);
    void skip_unexpected(const char*, bool verbose, const char*);
    void skip_unexpected(bool verbose=false, const char* msg="") {
        skip_unexpected(parsed_tag, verbose, msg);
    }
};

/////////////// START DEPRECATED XML PARSER
// Deprecated because it makes assumptions about
// the format of the XML being parsed
///////////////

// return true if the tag appears in the line
//
inline bool match_tag(const char* buf, const char* tag) {
    if (strstr(buf, tag)) return true;
    return false;
}

inline bool match_tag(const std::string &s, const char* tag) {
    return match_tag(s.c_str(), tag);
}

extern unsigned long long boinc_strtoull(const char *, char **, int);

// parse an integer of the form <tag>1234</tag>
// return true if it's there
// Note: this doesn't check for the end tag
//
inline bool parse_int(const char* buf, const char* tag, int& x) {
    const char* p = strstr(buf, tag);
    if (!p) return false;
    errno = 0;
    long y = strtol(p+strlen(tag), 0, 0);        // this parses 0xabcd correctly
    if (errno) return false;
    x = (int)y;
    return true;
}

// Same, for doubles
//
inline bool parse_double(const char* buf, const char* tag, double& x) {
    double y;
    const char* p = strstr(buf, tag);
    if (!p) return false;
    errno = 0;
#if (defined(__APPLE__) && defined(BUILDING_MANAGER))
// MacOS 13.3.1 apparently broke per-thread locale uselocale()
    y = strtod_l(p+strlen(tag), NULL, LC_C_LOCALE);
#else
    y = strtod(p+strlen(tag), NULL);
#endif
    if (errno) return false;
    if (!boinc_is_finite(y)) {
        return false;
    }
    x = y;
    return true;
}

extern bool parse(char* , char* );
extern bool parse_str(const char*, const char*, char*, int);
extern bool parse_str(const char* buf, const char* tag, std::string& dest);
extern void parse_attr(const char* buf, const char* attrname, char* out, int len);
extern bool parse_bool(const char*, const char*, bool&);

/////////////// END DEPRECATED XML PARSER

extern int copy_stream(FILE* in, FILE* out);
extern int strcatdup(char*& p, char* buf);
extern int dup_element_contents(FILE* in, const char* end_tag, char** pp);
extern int dup_element(FILE* in, const char* end_tag, char** pp);
extern int copy_element_contents(FILE* in, const char* end_tag, char* p, size_t len);
extern int copy_element_contents(FILE* in, const char* end_tag, std::string&);
extern void replace_element_contents(
    char* buf, const char* start, const char* end, const char* replacement
);
extern bool remove_element(char* buf, const char* start, const char* end);
extern bool str_replace(char* str, const char* old, const char* neww);
extern char* sgets(char* buf, int len, char* &in);
extern void non_ascii_escape(const char*, char*, int len);
extern void xml_escape(const char*, char*, int len);
extern void xml_unescape(std::string&);
extern void xml_unescape(char*);
extern void extract_venue(const char*, const char*, char*, int len);
extern int skip_unrecognized(char* buf, MIOFILE&);

#endif
