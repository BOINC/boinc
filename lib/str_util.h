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

#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#define KILO (1024.0)
#define MEGA (1048576.0)
#define GIGA (1024.*1048576.0)

extern int ndays_to_string(double x, int smallest_timescale, char *buf);
extern void nbytes_to_string(double nbytes, double total_bytes, char* str, int len);
extern int parse_command_line(char*, char**);
extern void c2x(char *what);
extern void strip_whitespace(char *str);
extern void strip_whitespace(std::string&);
#define safe_strcpy(x, y) strlcpy(x, y, sizeof(x))
#define safe_strcat(x, y) if (strlen(x)+strlen(y)<sizeof(x)) strcat(x, y)
extern char* time_to_string(double);
extern char* precision_time_to_string(double);
extern std::string timediff_format(double);

inline bool ends_with(std::string const& s, std::string const& suffix) {
    return
        s.size()>=suffix.size() &&
        s.substr(s.size()-suffix.size()) == suffix;
}

inline bool starts_with(std::string const& s, std::string const& prefix) {
    return s.substr(0, prefix.size()) == prefix;
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

// returns short text description of error corresponding to
// int errornumber from error_numbers.h
//
extern const char* boincerror(int which_error);
extern const char* network_status_string(int);
extern const char* rpc_reason_string(int);
extern const char* suspend_reason_string(int reason);


#endif
