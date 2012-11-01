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

// Scheduler code for directing a client to one of several
// download servers based on its time zone

#include "config.h"
#include <sys/param.h>
#include <string>
#include <cstdio>
#include <cstring>

#include "parse.h"

#include "sched_types.h"
#include "sched_msgs.h"
#include "sched_config.h"

#ifdef _USING_FCGI_
#include "boinc_fcgi.h"
#endif

typedef struct urltag {
    int zone;
    char name[124];
} URLTYPE;


// these global variables are needed to pass information into the
// compare function below.
//
static int tzone=0;
static int hostid=0;

// Evaluate differences between time-zone.  Two time zones that differ
// by almost 24 hours are actually very close on the surface of the
// earth.  This function finds the 'shortest way around'
//
static int compare(const void *x, const void *y) {
    const URLTYPE *a=(const URLTYPE *)x;
    const URLTYPE *b=(const URLTYPE *)y;

    char longname[512];

    const int twelve_hours = 12*3600;

    int diffa = abs(tzone - (a->zone));
    int diffb = abs(tzone - (b->zone));

    if (diffa > twelve_hours) {
        diffa = 2*twelve_hours-diffa;
    }

    if (diffb > twelve_hours) {
        diffb = 2*twelve_hours-diffb;
    }

    if (diffa < diffb) {
        return -1;
    }

    if (diffa > diffb) {
        return +1;
    }

    // In order to ensure uniform distribution, we hash paths that are
    // equidistant from the host's timezone in a way that gives a
    // unique ordering for each host but which is effectively random
    // between hosts.
    //
    sprintf(longname, "%s%d", a->name, hostid);
    std::string sa = md5_string((const unsigned char *)longname, strlen((const char *)longname));
    sprintf(longname, "%s%d", b->name, hostid);
    std::string sb = md5_string((const unsigned char *)longname, strlen((const char *)longname));
    int xa = strtol(sa.substr(1, 7).c_str(), 0, 16);
    int xb = strtol(sb.substr(1, 7).c_str(), 0, 16);

    if (xa<xb) {
        return -1;
    }

    if (xa>xb) {
        return 1;
    }

    return 0;
}

static URLTYPE *cached=NULL;
#define BLOCKSIZE 32

URLTYPE* read_download_list() {
    int count=0;
    int i;

    if (cached) return cached;

    const char *download_servers = config.project_path("download_servers");
#ifndef _USING_FCGI_
    FILE *fp=fopen(download_servers, "r");
#else
    FCGI_FILE *fp=FCGI::fopen(download_servers, "r");
#endif

    if (!fp) {
        log_messages.printf(MSG_CRITICAL,
            "File %s not found or unreadable!\n", download_servers
        );
        return NULL;
    }

    // read in lines from file
    //
    while (1) {
        // allocate memory in blocks
        if ((count % BLOCKSIZE)==0) {
            cached=(URLTYPE *)realloc(cached, (count+BLOCKSIZE)*sizeof(URLTYPE));
            if (!cached) {
                fclose(fp);
                return NULL;
            }
        }
        // read timezone offset and URL from file, and store in cache list
        //
        if (2==fscanf(fp, "%d %s", &(cached[count].zone), cached[count].name)) {
            count++;
        } else {
            // provide a null terminator so we don't need to keep
            // another global variable for count.
            //
            cached[count].name[0]='\0';
            break;
        }
    }
    fclose(fp);

    if (!count) {
        log_messages.printf(MSG_CRITICAL,
            "File %s contained no valid entries!\n"
            "Format of this file is one or more lines containing:\n"
            "TIMEZONE_OFFSET_IN_SEC   http://some.url.path\n",
            download_servers
        );
        free(cached);
        return NULL;
    }

    // sort URLs by distance from host timezone.  See compare() above
    // for details.
    qsort(cached, count, sizeof(URLTYPE), compare);

    log_messages.printf(MSG_DEBUG,
        "Sorted list of URLs follows [host timezone: UTC%+d]\n",
        tzone
    );
    for (i=0; i<count; i++) {
        log_messages.printf(MSG_DEBUG,
            "zone=%+06d url=%s\n", cached[i].zone, cached[i].name
        );
    }
    return cached;
}

// return number of bytes written, or <0 to indicate an error
//
int make_download_list(char *buffer, char *path, unsigned int lim, int tz) {
    char *start = buffer;
    unsigned int l,len = 0;
    int i;

    // global variable used in the compare() function
    tzone=tz;
    URLTYPE *serverlist=read_download_list();

    if (!serverlist) return -1;

    // print list of servers in sorted order.
    // Space is to format them nicely
    //
    for (i=0;
        strlen(serverlist[i].name) && (config.max_download_urls_per_file ?(i < config.max_download_urls_per_file) :true);
        i++
    ) {
        l = sprintf(start, "%s<url>%s%s</url>", i?"\n    ":"", serverlist[i].name, path);
        len += l;
        if (len >= lim) {
            *start = '\0';
            return (start-buffer);
        }
        start += l;
    }

    return (start-buffer);
}

// returns zero on success, non-zero to indicate an error
//
int add_download_servers(char *old_xml, char *new_xml, int tz) {
    char *p, *q, *r;
    int total_free = BLOB_SIZE - strlen(old_xml);

    p = (r = old_xml);

    // search for next URL to do surgery on
    while ((q=strstr(p, "<url>"))) {
        // p is at current position
        // q is at beginning of next "<url>" tag

        char *s;
        char path[MAXPATHLEN];
        int  len = q-p;

        // copy everything from p to q to new_xml
        //
        strncpy(new_xml, p, len);
        new_xml += len;

        // locate next instance of </url>
        //
        if (!(r = strstr(q, "</url>"))) {
            return 1;
        }
        r += strlen("</url>");

        // r points to the end of the whole "<url>...</url>" tag
        // parse out the URL into 'path'
        //
        if (!parse_str(q, "<url>", path, 1024)) {
            return 1;
        }

        // check if path contains the string specified in config.xml
        //
        if (!(s = strstr(path,config.replace_download_url_by_timezone))) {
            // if it doesn't, just copy the whole tag as it is
            strncpy(new_xml, q, r-q);
            new_xml += r-q;
            p=r;
        } else {
            // calculate free space available for URL replaces
            int lim = total_free - (len - (p - old_xml));

            // find end of the specified replace string,
            // i.e. start of the 'path'
            s += strlen(config.replace_download_url_by_timezone);

            // insert new download list in place of the original single URL
            len = make_download_list(new_xml, s, lim, tz);
            if (len == 0) {
                // if the replacement would exceed the maximum XML length,
                // just keep the original URL
                len = r-q;
                strncpy(new_xml, q, len);
            } else if (len < 0) {
                return 1;
            }
            new_xml += len;
            // advance pointer to start looking for next <url> tag.
            //
            p=r;
        }
    }

    strcpy(new_xml, r);
    return 0;
}

// replace the download URL for apps with a list of
// multiple download servers.
//
void process_av_timezone(APP_VERSION* avp, APP_VERSION& av2) {
    int retval;

    // set these global variables, needed by the compare()
    // function so that the download URL list can be sorted by timezone
    //
    tzone = g_reply->host.timezone;
    hostid = g_reply->host.id;
    retval = add_download_servers(avp->xml_doc, av2.xml_doc, g_reply->host.timezone);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_download_servers(to APP version) failed\n"
        );
        // restore original WU!
        av2 = *avp;
    }
}

// replace the download URL for WU files with a list of
// multiple download servers.
//
void process_wu_timezone(
    WORKUNIT& wu2, WORKUNIT& wu3
) {
    int retval;

    tzone = g_reply->host.timezone;
    hostid = g_reply->host.id;

    retval = add_download_servers(wu2.xml_doc, wu3.xml_doc, g_reply->host.timezone);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_download_servers(to WU) failed\n"
        );
        // restore original WU!
        wu3 = wu2;
    }
}

const char *BOINC_RCSID_28b6ac7093 = "$Id$";

