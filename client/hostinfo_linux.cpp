// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

#if   defined(_WIN32) && !defined(__STDWX_H__)
#include "boinc_win.h"
#elif defined(_WIN32) && defined(__STDWX_H__)
#include "stdwx.h"
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#endif

#include "str_replace.h"

#include "hostinfo.h"

bool HOST_INFO::parse_linux_os_info(FILE* file, const LINUX_OS_INFO_PARSER parser,
    char* os_name, const int os_name_size, char* os_version, const int os_version_size) {

    if (!file) {
        return false;
    }

    char buf[256];
    std::vector<std::string> lines;

    while (fgets(buf, 256, file)) {
        lines.push_back(buf);
    }

    return parse_linux_os_info(lines, parser, os_name, os_name_size, os_version, os_version_size);
}

bool HOST_INFO::parse_linux_os_info(const std::string& line, const LINUX_OS_INFO_PARSER parser,
    char* os_name, const int os_name_size, char* os_version, const int os_version_size) {
    if (line.empty()) {
        return false;
    }

    const char delim = '\n';

    return parse_linux_os_info(split(line, delim), parser, os_name, os_name_size, os_version, os_version_size);
}

bool HOST_INFO::parse_linux_os_info(const std::vector<std::string>& lines, const LINUX_OS_INFO_PARSER parser,
    char* os_name, const int os_name_size, char* os_version, const int os_version_size) {
    if (lines.empty()) {
        return false;
    }

    bool found_something = false;
    char buf[256], buf2[256];
    char dist_pretty[256], dist_name[256], dist_version[256], dist_codename[256];
    //string os_version_extra("");
    strcpy(dist_pretty, "");
    strcpy(dist_name, "");
    strcpy(dist_version, "");
    strcpy(dist_codename, "");

    switch (parser) {
    case lsbrelease: {
        for (unsigned int i = 0; i < lines.size(); ++i) {
            safe_strcpy(buf, lines[i].c_str());
            strip_whitespace(buf);
            if (strstr(buf, "Description:")) {
                found_something = true;
                safe_strcpy(dist_pretty, strchr(buf, ':') + 1);
                strip_whitespace(dist_pretty);
            }
            if (strstr(buf, "Distributor ID:")) {
                found_something = true;
                safe_strcpy(dist_name, strchr(buf, ':') + 1);
                strip_whitespace(dist_name);
            }
            if (strstr(buf, "Release:")) {
                found_something = true;
                safe_strcpy(dist_version, strchr(buf, ':') + 1);
                strip_whitespace(dist_version);
            }
            if (strstr(buf, "Codename:")) {
                found_something = true;
                safe_strcpy(dist_codename, strchr(buf, ':') + 1);
                strip_whitespace(dist_codename);
            }
        }
        break;
    }
    case osrelease: {
        for (unsigned int i = 0; i < lines.size(); ++i) {
            safe_strcpy(buf, lines[i].c_str());
            strip_whitespace(buf);
            // check if substr is at the beginning of the line
            if (strstr(buf, "PRETTY_NAME=") == buf) {
                found_something = true;
                safe_strcpy(buf2, strchr(buf, '=') + 1);
                strip_quotes(buf2);
                unescape_os_release(buf2);
                safe_strcpy(dist_pretty, buf2);
                continue;
            }
            if (strstr(buf, "NAME=") == buf) {
                found_something = true;
                safe_strcpy(buf2, strchr(buf, '=') + 1);
                strip_quotes(buf2);
                unescape_os_release(buf2);
                safe_strcpy(dist_name, buf2);
                continue;
            }
            if (strstr(buf, "VERSION=") == buf) {
                found_something = true;
                safe_strcpy(buf2, strchr(buf, '=') + 1);
                strip_quotes(buf2);
                unescape_os_release(buf2);
                safe_strcpy(dist_version, buf2);
                continue;
            }
            // could also be "UBUNTU_CODENAME="
            if (strstr(buf, "CODENAME=")) {
                found_something = true;
                safe_strcpy(buf2, strchr(buf, '=') + 1);
                strip_quotes(buf2);
                unescape_os_release(buf2);
                safe_strcpy(dist_codename, buf2);
                continue;
            }
        }
        break;
    }
    case redhatrelease: {
        safe_strcpy(buf, lines.front().c_str());
        found_something = true;
        strip_whitespace(buf);
        safe_strcpy(dist_pretty, buf);
        break;
    }
    default: {
        return false;
    }
    }

    if (found_something) {
        strcpy(buf2, "");
        if (strlen(dist_pretty)) {
            safe_strcat(buf2, dist_pretty);
        }
        else {
            if (strlen(dist_name)) {
                safe_strcat(buf2, dist_name);
                strcat(buf2, " ");
            }
            if (strlen(dist_version)) {
                safe_strcat(buf2, dist_version);
                strcat(buf2, " ");
            }
            if (strlen(dist_codename)) {
                safe_strcat(buf2, dist_codename);
                strcat(buf2, " ");
            }
            strip_whitespace(buf2);
        }
        strlcpy(os_version, buf2, os_version_size);
        if (strlen(dist_name)) {
            strlcpy(os_name, dist_name, os_name_size);
        }

        return true;
    }

    return false;
}
