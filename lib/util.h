// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
//
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License.
//
// The Original Code is the Berkeley Open Infrastructure for Network Computing.
//
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved.
//
// Contributor(s):
//

#ifndef UTIL_H
#define UTIL_H

#ifndef _WIN32
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cctype>
#include <cstring>
#include <cstdarg>
#include <algorithm>
#include <string>
using std::string;
#endif

extern int ndays_to_string(double x, int smallest_timescale, char *buf);
extern void nbytes_to_string(double nbytes, double total_bytes, char* str, int len);
extern double dtime();
extern void boinc_sleep(double);
extern int parse_command_line( char *, char ** );
extern int lock_file(char*);
extern void c2x(char *what);
extern void strip_whitespace(char *str);
extern void strip_whitespace(string&);
extern void unescape_url(char *url);
extern void escape_url(char *in, char*out);
extern void escape_url_readable(char* in, char* out);
extern void canonicalize_master_url(char *url);
extern void safe_strncpy(char*, const char*, int);
#define safe_strcpy(x, y) safe_strncpy(x, y, sizeof(x))
#define safe_strcat(x, y) if (strlen(x)+strlen(y)<sizeof(x)) strcat(x, y)
extern char* time_to_string(time_t);
extern char* timestamp();
string timediff_format(long tdiff);
int read_file_string(const char* pathname, string& result);

inline bool ends_with(string const& s, string const& suffix) {
    return
        s.size()>=suffix.size() &&
        s.substr(s.size()-suffix.size()) == suffix;
}

inline bool starts_with(string const& s, string const& prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

// http://lists.debian.org/debian-gcc/2002/debian-gcc-200204/msg00092.html
inline void downcase_string(
    string::iterator begin, string::iterator end, string::iterator src
) {
	std::transform(begin, end, src, (int(*)(int))tolower);
}

inline void downcase_string(string& w) {
    downcase_string(w.begin(), w.end(), w.begin());
}

// NOTE: use #include <functional>   to get max,min

// the __attribute((format...)) tags are GCC extensions that let the compiler
// do like-checking on printf-like arguments
#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x) /*nothing*/
#endif

// See lib/messages.C for commentary
class Messages {
    int debug_level;
    int indent_level;
    char spaces[80];
    FILE* output;
public:

    Messages(FILE* output);
    void enter_level(int = 1);
    void leave_level() { enter_level(-1); }
    Messages& operator++() { enter_level(); return *this; }
    Messages& operator--() { leave_level(); return *this; }

    void printf(int kind, const char* format, ...) __attribute__ ((format (printf, 3, 4)));
    void printf_multiline(int kind, const char* str, const char* prefix_format, ...) __attribute__ ((format (printf, 4, 5)));
    void printf_file(int kind, const char* filename, const char* prefix_format, ...) __attribute__ ((format (printf, 4, 5)));
    void vprintf(int kind, const char* format, va_list va);
    void vprintf_multiline(int kind, const char* str, const char* prefix_format, va_list va);
    void vprintf_file(int kind, const char* filename, const char* prefix_format, va_list va);

protected:

    virtual const char* v_format_kind(int kind) const = 0;
    virtual bool v_message_wanted(int kind) const = 0;
};

// automatically ++/--Messages on scope entry / exit. See lib/messages.C for commentary
class ScopeMessages
{
    Messages& messages;
    int kind;
public:
    ScopeMessages(Messages& messages_, int kind_) : messages(messages_), kind(kind_)
    { ++messages; }
    ~ScopeMessages() { --messages; }
    ScopeMessages& operator++() { ++messages; return *this; }
    ScopeMessages& operator--() { --messages; return *this; }

    void printf(const char* format, ...) __attribute__ ((format (printf, 2, 3)));
    void printf_multiline(const char* str, const char* prefix_format, ...) __attribute__ ((format (printf, 3, 4)));
    void printf_file(const char* filename, const char* prefix_format, ...) __attribute__ ((format (printf, 3, 4)));
};


#define SECONDS_PER_DAY 86400

static inline double drand() {
    return (double)rand()/(double)RAND_MAX;
}

// return a random integer in the range [rmin,rmax)
static inline double rand_range(double rmin, double rmax)
{
    if (rmin < rmax)
        return drand() * (rmax-rmin) + rmin;
    else
        return rmin;
}

// return a random integer in the range [MIN,min(e^n,MAX))
int calculate_exponential_backoff(
    const char* debug_descr, int n, double MIN, double MAX,
    double factor=1.0
);
extern bool debug_fake_exponential_backoff;


#ifdef _WIN32

char* windows_error_string( char* pszBuf, int iSize );
char* windows_format_error_string( unsigned long dwError, char* pszBuf, int iSize );

#endif

#endif
