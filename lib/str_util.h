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

#ifndef BOINC_STR_UTIL_H
#define BOINC_STR_UTIL_H

#include <string>
#include <vector>
#include <string.h>

#include "str_replace.h"

#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))
#define safe_strcat(x, y) strlcat(x, y, sizeof(x))

extern void strcpy_overlap(char*, const char*);
extern int ndays_to_string(double x, int smallest_timescale, char *buf);
extern void nbytes_to_string(double nbytes, double total_bytes, char* str, int len);
extern int parse_command_line(char*, char**);
extern void strip_whitespace(char *str);
extern void strip_whitespace(std::string&);
extern void strip_quotes(char *str);
extern void strip_quotes(std::string&);
extern void unescape_os_release(char *str);
extern void collapse_whitespace(char *str);
extern void collapse_whitespace(std::string&);
extern char* time_to_string(double);
extern char* precision_time_to_string(double);
extern void secs_to_hmsf(double, char*);
extern std::string timediff_format(double);

inline bool empty(char* p) {
    return p[0] == 0;
}

inline bool ends_with(std::string const& s, std::string const& suffix) {
    return
        s.size()>=suffix.size() &&
        s.substr(s.size()-suffix.size()) == suffix;
}

inline bool ends_with(const char* s, const char* suffix) {
    int m = (int)strlen(s), n = (int)strlen(suffix);
    if (n > m) return false;
    return (strcmp(suffix, s+m-n) == 0);
}

inline bool starts_with(std::string const& s, std::string const& prefix) {
    return s.substr(0, prefix.size()) == prefix;
}

inline bool starts_with(const char* s, const char* prefix) {
    return (strncmp(s, prefix, strlen(prefix)) == 0);
}

inline void downcase_string(std::string& w) {
    for (std::string::iterator p = w.begin(); p != w.end(); ++p) {
        *p = (char)tolower((int)*p);
    }
}

inline void downcase_string(char* p) {
    while (*p) {
        *p = (char)tolower((int)*p);
        p++;
    }
}

extern int string_substitute(
    const char* haystack, char* out, int out_len,
    const char* needle, const char* target
);

// convert UNIX time to MySQL timestamp (yyyymmddhhmmss)
//
extern void mysql_timestamp(double, char*);

// parse host.serialnum into component parts.
// Given a string of the form
// [BOINC|7.2.42][CUDA|GeForce GTX 860M|1|2048MB|34052|101][INTEL|Intel(R) HD Graphics 4600|1|1752MB||102][vbox|4.2.16]
// split it into the BOINC, vbox, and other (coproc) parts
//
extern void parse_serialnum(char* in, char* boinc, char* vbox, char* coprocs);

// take a malloced string.
// if \n is not last char, add it.
//
extern char* lf_terminate(char*);

extern const char* network_status_string(int);
extern const char* rpc_reason_string(int);
extern const char* suspend_reason_string(int reason);
extern const char* run_mode_string(int mode);
extern const char* battery_state_string(int state);
extern const char* result_client_state_string(int state);
extern const char* result_scheduler_state_string(int state);
extern const char* active_task_state_string(int state);
extern const char* batch_state_string(int state);

extern void strip_translation(char* p);

extern std::vector<std::string> split(std::string, char delim);

extern bool is_valid_filename(const char*);

extern int path_to_filename(std::string fpath, std::string& fname);
extern int path_to_filename(std::string fpath, char* &fname);
#endif
