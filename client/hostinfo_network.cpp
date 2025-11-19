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
#else
#include "config.h"
#include <cstdio>
#include <cstring>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "error_numbers.h"
#include "file_names.h"
#include "mac_address.h"
#include "parse.h"
#include "str_replace.h"
#include "str_util.h"
#include "util.h"

#include "client_msgs.h"
#include "client_state.h"

#include "hostinfo.h"

// get domain name and IP address of this host
// Android: if domain_name is empty, set it to android_xxxxxxxx
//
int HOST_INFO::get_local_network_info() {
    safe_strcpy(ip_addr, "");

#ifdef ANDROID
    if (strlen(domain_name) && strcmp(domain_name, "localhost")) return 0;
    char buf[256];
    make_random_string("", buf);
    buf[8] = 0;
    snprintf(domain_name, sizeof(domain_name), "android_%s", buf);
    return 0;
#endif

    struct sockaddr_storage s;

    safe_strcpy(domain_name, "");

    // it seems like we should use getdomainname() instead of gethostname(),
    // but on FC6 it returns "(none)".
    //
    if (gethostname(domain_name, 256)) {
        return ERR_GETHOSTBYNAME;
    }
    int retval = resolve_hostname(domain_name, s);
    if (retval) return retval;
#ifdef _WIN32
    sockaddr_in* sin = (sockaddr_in*)&s;
    strlcpy(ip_addr, inet_ntoa(sin->sin_addr), sizeof(ip_addr));
#else
    if (s.ss_family == AF_INET) {
        sockaddr_in* sin = (sockaddr_in*)&s;
        inet_ntop(AF_INET, (void*)(&sin->sin_addr), ip_addr, 256);
    } else {
        sockaddr_in6* sin = (sockaddr_in6*)&s;
        inet_ntop(AF_INET6, (void*)(&sin->sin6_addr), ip_addr, 256);
    }
#endif
    if (!cc_config.device_name.empty()) {
        safe_strcpy(domain_name, cc_config.device_name.c_str());
    }
    return 0;
}

// make a random string using time of day and host info.
// Not recommended for password generation;
// use as a last resort if more secure methods fail
//
void HOST_INFO::make_random_string(const char* salt, char* out) {
    char buf[1024];

#ifdef ANDROID
    snprintf(buf, sizeof(buf),
        "%f%s%s%f%s",
        dtime(), domain_name, ip_addr, d_free, salt
    );
#else
    snprintf(buf, sizeof(buf),
        "%d%.15e%s%s%f%s",
        getpid(), dtime(), domain_name, ip_addr, d_free, salt
    );
#endif
    md5_block((const unsigned char*) buf, (int)strlen(buf), out);
}

void make_secure_random_string(char* out) {
    int retval = make_secure_random_string_os(out);
    if (retval) {
        if (cc_config.os_random_only) {
            msg_printf(
                NULL, MSG_INTERNAL_ERROR,
                "OS random string generation failed, exiting"
            );
            exit(1);
        }
        gstate.host_info.make_random_string("guirpc", out);
    }
}

// make a host cross-project ID.
// Should be unique across hosts with very high probability
//
void HOST_INFO::generate_host_cpid() {
    int retval;
    char buf[256+MAXPATHLEN];
    char dir[MAXPATHLEN];

    // if a MAC address is available, compute an ID based on it;
    // this has the advantage of stability
    // (a given host will get the same ID each time BOINC is reinstalled)
    //
    retval = get_mac_address(buf);
    if (retval) {
        make_random_string("", host_cpid);
        return;
    }

    // append the current dir to the MAC address;
    // that way if there are multiple instances per host
    // (used by some grid installations)
    // the instances will get different CPIDs
    //
    boinc_getcwd(dir);
    safe_strcat(buf, dir);
    md5_block((const unsigned char*) buf, (int)strlen(buf), host_cpid);
}
