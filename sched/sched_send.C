// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

// scheduler code related to sending work

#include <vector>
#include <string>
#include <ctime>
#include <cstdio>
#include <stdlib.h>

using namespace std;

#include "error_numbers.h"
#include "server_types.h"
#include "sched_shmem.h"
#include "sched_config.h"
#include "sched_util.h"
#include "main.h"
#include "sched_msgs.h"
#include "sched_send.h"
#include "sched_locality.h"
#include "../lib/parse.h"

#ifdef _USING_FCGI_
#include "fcgi_stdio.h"
#endif

const int MIN_SECONDS_TO_SEND = 0;
const int MAX_SECONDS_TO_SEND = (28*SECONDS_IN_DAY);

const double MIN_POSSIBLE_RAM = 64000000;

bool anonymous(PLATFORM& platform) {
    return (!strcmp(platform.name, "anonymous"));
}

bool SCHEDULER_REQUEST::has_version(APP& app) {
    unsigned int i;

    for (i=0; i<client_app_versions.size(); i++) {
        CLIENT_APP_VERSION& cav = client_app_versions[i];
        if (!strcmp(cav.app_name, app.name) && cav.version_num >= app.min_version) {
            return true;
        }
    }
    return false;
}

// This is an ugly way to keep track of *why* a particular host didn't
// get its work request satisfied.  Unfortunately I don't see a clean
// way of doing this without global vars.  David, Rom?
//

// Initialized to zero, since it's static memory.
double watch_diskspace[3];

// compute the max additional disk usage we can impose on the host
//
double max_allowable_disk(SCHEDULER_REQUEST& req) {
// ROMW: Reverting back to older implementation until all clients are 4.x
//       or higher.
#if 1
    HOST host = req.host;
    GLOBAL_PREFS prefs = req.global_prefs;
    double x1, x2, x3, x;

    // fill in default values for missing prefs
    //
    if (prefs.disk_max_used_gb == 0) prefs.disk_max_used_gb = 0.1;   // 100 MB
    if (prefs.disk_max_used_pct == 0) prefs.disk_max_used_pct = 10;
    // min_free_gb can be zero

    // default values for BOINC disk usage (project and total) is zero
    //

    // no defaults for total/free disk space (host.d_total, d_free)
    // if they're zero, project will get no work.
    //

    x1 = prefs.disk_max_used_gb*1e9 - req.total_disk_usage;
    x2 = host.d_total*prefs.disk_max_used_pct/100.;
    x3 = host.d_free - prefs.disk_min_free_gb*1e9;      // may be negative

    x = min(x1, min(x2, x3));

    // keep track of which bound is the most stringent
    if (x==x1) {
        watch_diskspace[0]=x;
    } else if (x==x2) {
        watch_diskspace[1]=x;
    } else {
        watch_diskspace[2]=x;
	}

    if (x < 0) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "disk_max_used_gb %f disk_max_used_pct %f disk_min_free_gb %f\n",
            prefs.disk_max_used_gb, prefs.disk_max_used_pct,
            prefs.disk_min_free_gb
        );
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "req.total_disk_usage %f host.d_total %f host.d_free %f\n",
            req.total_disk_usage, host.d_total, host.d_free
        );
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "x1 %f x2 %f x3 %f x %f\n",
            x1, x2, x3, x
        );
    }
    return x;
#else
    double x1, x2, x3;

    HOST host = req.host;

    x1 = req.project_disk_free;
    x2 = req.potentially_free_offender;
    x3 = req.potentially_free_self;

    if (x1 < 0) {
        log_messages.printf(
            SCHED_MSG_LOG::NORMAL,
            "req.project_disk_free_gb %f\n",
            x1
        );
    }
    return max(max(x1,x2), x3);
#endif
}

// if a host has active_frac < 0.1, assume 0.1 so we don't deprive it of work.
//
const double HOST_ACTIVE_FRAC_MIN = 0.1;

// estimate the number of CPU seconds that a workunit requires
// running on this host.
//
static double estimate_cpu_duration(WORKUNIT& wu, HOST& host) {
    if (host.p_fpops <= 0) host.p_fpops = 1e9;
    if (wu.rsc_fpops_est <= 0) wu.rsc_fpops_est = 1e12;
    return wu.rsc_fpops_est/host.p_fpops;
}

// estimate the amount of real time to complete this WU,
// taking into account active_frac and resource_share_fraction
//
static double estimate_wallclock_duration(
    WORKUNIT& wu, HOST& host, double resource_share_fraction
) {
	double running_frac = host.active_frac * host.on_frac;
	if (running_frac < HOST_ACTIVE_FRAC_MIN) running_frac = HOST_ACTIVE_FRAC_MIN;
	if (running_frac > 1) running_frac = 1;
    double ecd = estimate_cpu_duration(wu, host);
    double ewd = ecd/(running_frac*resource_share_fraction);
#if 0
    log_messages.printf(
        SCHED_MSG_LOG::DEBUG, "est cpu dur %f; running_frac %f; rsf %f; est %f\n",
        ecd, running_frac, resource_share_fraction, ewd
    );
#endif

    return ewd;
}

// if the WU can't be executed on the host, return a bitmap of reasons why.
// Reasons include:
// 1) the host doesn't have enough memory;
// 2) the host doesn't have enough disk space;
// 3) based on CPU speed, resource share and estimated delay,
//    the host probably won't get the result done within the delay bound
//
// NOTE: This is a "fast" check; no DB access allowed.
// In particular it doesn't enforce the one-result-per-user-per-wu rule
//
int wu_is_infeasible(
    WORKUNIT& wu, SCHEDULER_REQUEST& request, SCHEDULER_REPLY& reply
) {
    int reason = 0;

    double m_nbytes = reply.host.m_nbytes;
    if (m_nbytes < MIN_POSSIBLE_RAM) m_nbytes = MIN_POSSIBLE_RAM;

    if (wu.rsc_memory_bound > m_nbytes) {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "[WU#%d %s] needs %f mem; [HOST#%d] has %f\n",
            wu.id, wu.name, wu.rsc_memory_bound, reply.host.id, m_nbytes
        );
        reply.wreq.insufficient_mem = true;
        reason |= INFEASIBLE_MEM;
    }

    if (wu.rsc_disk_bound > reply.wreq.disk_available) {
        reply.wreq.insufficient_disk = true;
        reason |= INFEASIBLE_DISK;
    }

	// TODO: take into account delay due to other results
	// being sent in the current RPC reply
	//
    if (config.enforce_delay_bound) {
        double wu_wallclock_time = estimate_wallclock_duration(
            wu, reply.host, request.resource_share_fraction
        );
        double host_remaining_time = request.estimated_delay;

        if (host_remaining_time + wu_wallclock_time > wu.delay_bound) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG,
                "[WU#%d %s] needs %d seconds on [HOST#%d]; delay_bound is %d\n",
                wu.id, wu.name, (int)wu_wallclock_time, reply.host.id,
                wu.delay_bound
            );
            reply.wreq.insufficient_speed = true;
            reason |= INFEASIBLE_CPU;
        }
    }

    return reason;
}

// insert "text" right after "after" in the given buffer
//
int insert_after(char* buffer, char* after, char* text) {
    char* p;
    char temp[LARGE_BLOB_SIZE];

    if (strlen(buffer) + strlen(text) > LARGE_BLOB_SIZE-1) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "insert_after: overflow\n");
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(SCHED_MSG_LOG::CRITICAL, "insert_after: %s not found in %s\n", after, buffer);
        return ERR_NULL;
    }
    p += strlen(after);
    strcpy(temp, p);
    strcpy(p, text);
    strcat(p, temp);
    return 0;
}

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
    URLTYPE *a=(URLTYPE *)x;
    URLTYPE *b=(URLTYPE *)y;
    
    char longname[512];
    
    const int twelve_hours = 12*3600;

    int diffa = abs(tzone - (a->zone));
    int diffb = abs(tzone - (b->zone));

    if (diffa > twelve_hours)
        diffa = 2*twelve_hours-diffa;
    
    if (diffb > twelve_hours)
        diffb = 2*twelve_hours-diffb;

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
    FILE *fp;
    int count=0;
    int i;
    
    if (cached) return cached;
    
    if (!(fp=fopen("../download_servers", "r"))) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "File ../download_servers not found or unreadable!\n"
        );
        return NULL;
    }

    // read in lines from file
    while (1) {
        // allocate memory in blocks
        if ((count % BLOCKSIZE)==0) {
            cached=(URLTYPE *)realloc(cached, (count+BLOCKSIZE)*sizeof(URLTYPE));
            if (!cached) return NULL;
        }
        // read timezone offset and URL from file, and store in cache
        // list
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
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "File ../download_servers contained no valid entries!\n"
            "Format of this file is one or more lines containing:\n"
            "TIMEZONE_OFFSET_IN_SEC   http://some.url.path\n"
        );
        free(cached);
        return NULL;
    }
    
    // sort URLs by distance from host timezone.  See compare() above
    // for details.
    qsort(cached, count, sizeof(URLTYPE), compare);
    
    log_messages.printf(
        SCHED_MSG_LOG::DEBUG, "Sorted list of URLs follows\n" 
    );
    for (i=0; i<count; i++) {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG, "zone=%d name=%s\n", cached[i].zone, cached[i].name
        );
    }
    return cached;
}

// return number of bytes written, or <0 to indicate an error
//
int make_download_list(char *buffer, char *path, int timezone) {
    char *start=buffer;
    int i;

    // global variable used in the compare() function
    tzone=timezone;
    URLTYPE *serverlist=read_download_list();
    
    if (!serverlist) return -1;
    
    // print list of servers in sorted order.
	// Space is to format them nicely
	//
    for (i=0; strlen(serverlist[i].name); i++) {
        start+=sprintf(start, "%s<url>%s/%s</url>", i?"\n    ":"", serverlist[i].name, path);
	}

    // make a second copy in the same order
	//
    for (i=0; strlen(serverlist[i].name); i++) {
        start+=sprintf(start, "%s<url>%s/%s</url>", "\n    ", serverlist[i].name, path);
	}
    
    return (start-buffer);
}

// returns zero on success, non-zero to indicate an error
//
int add_download_servers(char *old_xml, char *new_xml, int timezone) {
    char *p, *q, *r;
    
    p=r=old_xml;

    // search for next URL to do surgery on 
    while ((q=strstr(p, "<url>"))) {
        char *s;
        char path[1024];
        int len = q-p;
        
        strncpy(new_xml, p, len);
        
        new_xml += len;
        
        // locate next instance of </url>
		//
        if (!(r=strstr(q, "</url>"))) {
            return 1;
		}
        r += strlen("</url>");
        
        // parse out the URL
		//
        if (!parse_str(q, "<url>", path, 1024)) {
            return 1;
		}
        
        // find start of 'download/'
		//
        if (!(s=strstr(path,"download/"))) {
            return 1;
		}
        
        // insert new download list in place of the original one
		//
        len = make_download_list(new_xml, s, timezone);
        if (len<0) {
            return 1;
		}
        new_xml += len;
        
        // advance pointer to start looking for next <url> tag.
		//
        p=r;
    }
    
    strcpy(new_xml, r);
    return 0;
}

// add elements to WU's xml_doc,
// in preparation for sending it to a client
//
int insert_wu_tags(WORKUNIT& wu, APP& app) {
    char buf[LARGE_BLOB_SIZE];
    
    sprintf(buf,
        "    <rsc_fpops_est>%f</rsc_fpops_est>\n"
        "    <rsc_fpops_bound>%f</rsc_fpops_bound>\n"
        "    <rsc_memory_bound>%f</rsc_memory_bound>\n"
        "    <rsc_disk_bound>%f</rsc_disk_bound>\n"
        "    <name>%s</name>\n"
        "    <app_name>%s</app_name>\n",
        wu.rsc_fpops_est,
        wu.rsc_fpops_bound,
        wu.rsc_memory_bound,
        wu.rsc_disk_bound,
        wu.name,
        app.name
    );
    return insert_after(wu.xml_doc, "<workunit>\n", buf);
}

// return the APP and APP_VERSION for the given WU, for the given platform.
// return false if none
//
bool find_app_version(
    WORK_REQ& wreq, WORKUNIT& wu, PLATFORM& platform, SCHED_SHMEM& ss,
    APP*& app, APP_VERSION*& avp
) {
    app = ss.lookup_app(wu.appid);
    if (!app) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL, "Can't find APP#%d\n", wu.appid
        );
        return false;
    }
    avp = ss.lookup_app_version(app->id, platform.id, app->min_version);
    if (!avp) {
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "no app version available: APP#%d PLATFORM#%d min_version %d\n",
            app->id, platform.id, app->min_version
        );
        wreq.no_app_version = true;
        return false;
    }
    return true;
}

// verify that the given APP_VERSION will work with the core client
//
bool app_core_compatible(WORK_REQ& wreq, APP_VERSION& av) {
    if (wreq.core_client_version < av.min_core_version) {
#if 0
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "Outdated core version: wanted %d, got %d\n",
            av.min_core_version, wreq.core_client_version
        );
#endif
        wreq.outdated_core = true;
        return false;
    }
    return true;
}

// add the given workunit to a reply.
// look up its app, and make sure there's a version for this platform.
// Add the app and app_version to the reply also.
//
int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY& reply, PLATFORM& platform,
    APP* app, APP_VERSION* avp
) {
    int retval;
    WORKUNIT wu2, wu3;
    
    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    if (avp) {
        APP_VERSION av2=*avp, *avp2=&av2;
        
        if (config.choose_download_url_by_timezone) {
            // replace the download URL for apps with a list of
            // multiple download servers.

            // set these global variables, needed by the compare()
            // function so that the download URL list can be sorted by
            // timezone
            tzone=reply.host.timezone;
            hostid=reply.host.id;
            retval = add_download_servers(avp->xml_doc, av2.xml_doc, reply.host.timezone);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "add_download_servers(to APP version) failed\n"
                );
                // restore original WU!
                av2=*avp;
            }
        }
        
        reply.insert_app_unique(*app);
        reply.insert_app_version_unique(*avp2);
        log_messages.printf(
            SCHED_MSG_LOG::DEBUG,
            "[HOST#%d] Sending app_version %s %s %d\n",
            reply.host.id, app->name, platform.name, avp2->version_num
        );
    }

    // add time estimate to reply
    //
    wu2 = wu;       // make copy since we're going to modify its XML field
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(SCHED_MSG_LOG::NORMAL, "insert_wu_tags failed\n");
        return retval;
    }
    wu3=wu2;
    if (config.choose_download_url_by_timezone) {
        // replace the download URL for WU files with a list of
        // multiple download servers.
        
        // set these global variables, needed by the compare()
        // function so that the download URL list can be sorted by
        // timezone
        tzone=reply.host.timezone;
        hostid=reply.host.id;
        
        retval = add_download_servers(wu2.xml_doc, wu3.xml_doc, reply.host.timezone);
        if (retval) {
            log_messages.printf(
                SCHED_MSG_LOG::CRITICAL,
                "add_download_servers(to WU) failed\n"
            );
            // restore original WU!
            wu3=wu2;
        }
    }
    
    reply.insert_workunit_unique(wu3);
    return 0;
}

int insert_name_tags(RESULT& result, WORKUNIT const& wu) {
    char buf[256];
    int retval;

    sprintf(buf, "<name>%s</name>\n", result.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    sprintf(buf, "<wu_name>%s</wu_name>\n", wu.name);
    retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

int insert_deadline_tag(RESULT& result) {
    char buf[256];
    sprintf(buf, "<report_deadline>%d</report_deadline>\n", result.report_deadline);
    int retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

static int update_wu_transition_time(WORKUNIT wu, time_t x) {
    DB_WORKUNIT dbwu;
    char buf[256];

    dbwu.id = wu.id;
    sprintf(buf, "transition_time=%d", (int)x);
    return dbwu.update_field(buf);
}

// return true iff a result for same WU is already being sent
//
static bool wu_already_in_reply(WORKUNIT& wu, SCHEDULER_REPLY& reply) {
    unsigned int i;
    for (i=0; i<reply.results.size(); i++) {
        if (wu.id == reply.results[i].workunitid) {
            return true;
        }
    }
    return false;
}

// modified by Pietro Cicotti
// Check that the two platform has the same architecture and operating system
// Architectures: AMD, Intel, Macintosh
// OS: Linux, Windows, Darwin, SunOS

const int unspec = 0;
const int nocpu = 1;
const int Intel = 2;
const int AMD = 3;
const int Macintosh = 4;

const int noos = 128;
const int Linux = 256;
const int Windows = 384;
const int Darwin = 512;
const int SunOS = 640;

inline
int OS(SCHEDULER_REQUEST& sreq){
    if ( strstr(sreq.host.os_name, "Linux") != NULL ) return Linux;
    else if( strstr(sreq.host.os_name, "Windows") != NULL ) return Windows;
    else if( strstr(sreq.host.os_name, "Darwin") != NULL ) return Darwin;
    else if( strstr(sreq.host.os_name, "SunOS") != NULL ) return SunOS;
    else return noos;
};

inline
int CPU(SCHEDULER_REQUEST& sreq){
    if ( strstr(sreq.host.p_vendor, "Intel") != NULL ) return Intel;
    else if( strstr(sreq.host.p_vendor, "AMD") != NULL ) return AMD;
    else if( strstr(sreq.host.p_vendor, "Macintosh") != NULL ) return Macintosh;
    else return nocpu;
};

#if 0
// old version, just in case
bool same_platform(DB_HOST& host, SCHEDULER_REQUEST& sreq) {
    return !strcmp(host.os_name, sreq.host.os_name)
        && !strcmp(host.p_vendor, sreq.host.p_vendor);
}
#endif

// return true if we've already sent a result of this WU to a different platform
// (where "platform" is os_name + p_vendor;
// may want to sharpen this for Unix)
//
static bool already_sent_to_different_platform(
    SCHEDULER_REQUEST& sreq, WORKUNIT& workunit, WORK_REQ& wreq
) {
    DB_WORKUNIT db_wu;
    int retval, hr_class=0;
    char buf[256];

    // reread hr_class field from DB in case it's changed
    //
    db_wu.id = workunit.id;
    retval = db_wu.get_field_int("hr_class", hr_class);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL, "can't get hr_class for %d: %d\n",
            db_wu.id, retval
        );
        return true;
    }
    wreq.homogeneous_redundancy_reject = false;
    if (hr_class != unspec) {
        if (OS(sreq) + CPU(sreq) != hr_class) {
            wreq.homogeneous_redundancy_reject = true;
        }
    } else {
        hr_class = OS(sreq) + CPU(sreq);
        sprintf(buf, "hr_class=%d", hr_class);
        db_wu.update_field(buf);
    }
    return wreq.homogeneous_redundancy_reject;
}

void lock_sema() {
    lock_semaphore(sema_key);
}

void unlock_sema() {
    unlock_semaphore(sema_key);
}

// return true if additional work is needed,
// and there's disk space left,
// and we haven't exceeded result per RPC limit,
// and we haven't exceeded results per day limit
//
bool SCHEDULER_REPLY::work_needed() {
    if (wreq.seconds_to_fill <= 0) return false;
    if (wreq.disk_available <= 0) {
        wreq.insufficient_disk = true;
        return false;
    }
    if (wreq.nresults >= config.max_wus_to_send) return false;
    if (config.daily_result_quota) {
        if (host.nresults_today >= config.daily_result_quota) {
            wreq.daily_result_quota_exceeded = true;
            return false;
        }
    }
    return true;
}

int add_result_to_reply(
    DB_RESULT& result, WORKUNIT& wu, SCHEDULER_REQUEST& request,
    SCHEDULER_REPLY& reply, PLATFORM& platform,
    APP* app, APP_VERSION* avp
) {
    int retval;
    double wu_seconds_filled;

    retval = add_wu_to_reply(wu, reply, platform, app, avp);
    if (retval) return retval;

    // If using locality scheduling, there are probably many
    // result that use same file, so don't decrement available space
    //
    if (!config.locality_scheduling) {
        reply.wreq.disk_available -= wu.rsc_disk_bound;
    }

    // update the result in DB
    //
    result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    result.hostid = reply.host.id;
    result.userid = reply.user.id;
    result.sent_time = time(0);
    result.report_deadline = result.sent_time + wu.delay_bound;
    result.update_subset();

    wu_seconds_filled = estimate_wallclock_duration(
        wu, reply.host, request.resource_share_fraction
    );
    log_messages.printf(
        SCHED_MSG_LOG::NORMAL,
        "[HOST#%d] Sending [RESULT#%d %s] (fills %.2f seconds)\n",
        reply.host.id, result.id, result.name, wu_seconds_filled
    );

    retval = update_wu_transition_time(wu, result.report_deadline);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "send_work: can't update WU transition time\n"
        );
    }

    // The following overwrites the result's xml_doc field.
    // But that's OK cuz we're done with DB updates
    //
    retval = insert_name_tags(result, wu);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL, "send_work: can't insert name tags\n"
        );
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(
            SCHED_MSG_LOG::CRITICAL,
            "send_work: can't insert deadline tag\n"
        );
    }
    reply.insert_result(result);
    reply.wreq.seconds_to_fill -= wu_seconds_filled;
    reply.wreq.nresults++;
    reply.host.nresults_today++;
    return 0;
}

// Make a pass through the wu/results array, sending work.
// If "infeasible_only" is true, send only results that were
// previously infeasible for some host
//
static void scan_work_array(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
    int i, j, retval, n, rnd_off;
    WORKUNIT wu;
    DB_RESULT result;
    char buf[256];
    APP* app;
    APP_VERSION* avp;
    bool found;

    lock_sema();
    
    rnd_off = rand() % ss.nwu_results;
    for (j=0; j<ss.nwu_results; j++) {
        i = (j+rnd_off) % ss.nwu_results;
        if (!reply.work_needed()) break;

        WU_RESULT& wu_result = ss.wu_results[i];

        // do fast checks on this wu_result;
        // i.e. ones that don't require DB access
        // if any check fails, continue

        if (wu_result.state != WR_STATE_PRESENT && wu_result.state != g_pid) {
            continue;
        }

        if (reply.wreq.infeasible_only && (wu_result.infeasible_count==0)) {
            continue;
        }

        // don't send if we're already sending a result for same WU
        //
        if (config.one_result_per_user_per_wu) {
            if (wu_already_in_reply(wu_result.workunit, reply)) {
                continue;
            }
        }

        // don't send if host can't handle it
        //
        wu = wu_result.workunit;
        if (wu_is_infeasible(wu, sreq, reply)) {
            log_messages.printf(
                SCHED_MSG_LOG::DEBUG, "[HOST#%d] [WU#%d %s] WU is infeasible\n",
                reply.host.id, wu.id, wu.name
            );
            wu_result.infeasible_count++;
            continue;
        }

        // Find the app and app_version for the client's platform.
        // If none, treat the WU as infeasible
        //
        if (anonymous(platform)) {
            app = ss.lookup_app(wu.appid);
            found = sreq.has_version(*app);
            if (!found) {
                continue;
            }
            avp = NULL;
        } else {
            found = find_app_version(reply.wreq, wu, platform, ss, app, avp);
            if (!found) {
                wu_result.infeasible_count++;
                continue;
            }

            // see if the core client is too old.
            // don't bump the infeasible count because this
            // isn't the result's fault
            //
            if (!app_core_compatible(reply.wreq, *avp)) {
                continue;
            }
        }

        // end of fast checks - mark wu_result as checked out and release sema.
        // from here on in this loop, don't continue on failure;
        // instead, goto dont_send (so that we reacquire semaphore)

        wu_result.state = WR_STATE_CHECKED_OUT;
        unlock_sema();

        // Don't send if we've already sent a result of this WU to this user.
        //
        if (config.one_result_per_user_per_wu) {
            sprintf(buf,
                "where workunitid=%d and userid=%d",
                wu_result.workunit.id, reply.user.id
            );
            retval = result.count(n, buf);
            if (retval) {
                log_messages.printf(
                    SCHED_MSG_LOG::CRITICAL,
                    "send_work: can't get result count (%d)\n", retval
                );
                goto dont_send;
            } else {
                if (n>0) {
                    log_messages.printf(
                        SCHED_MSG_LOG::DEBUG,
                        "send_work: user %d already has %d result(s) for WU %d\n",
                        reply.user.id, n, wu_result.workunit.id
                    );
                    goto dont_send;
                }
            }
        }

        // if desired, make sure redundancy is homogeneous
        //
        if (config.homogeneous_redundancy || app->homogeneous_redundancy) {
            if (already_sent_to_different_platform(
                sreq, wu_result.workunit, reply.wreq
            )) {
                goto dont_send;
            }
        }

        result.id = wu_result.resultid;

        // mark slot as empty AFTER we've copied out of it
        // (since otherwise feeder might overwrite it)
        //
        wu_result.state = WR_STATE_EMPTY;

        // reread result from DB, make sure it's still unsent
        // TODO: from here to update() should be a transaction
        //
        retval = result.lookup_id(result.id);
        if (retval) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "[RESULT#%d] result.lookup_id() failed %d\n",
                result.id, retval
            );
            goto done;
        }
        if (result.server_state != RESULT_SERVER_STATE_UNSENT) {
            log_messages.printf(SCHED_MSG_LOG::DEBUG,
                "[RESULT#%d] expected to be unsent; instead, state is %d\n",
                result.id, result.server_state
            );
            goto done;
        }
        if (result.workunitid != wu.id) {
            log_messages.printf(SCHED_MSG_LOG::CRITICAL,
                "[RESULT#%d] wrong WU ID: wanted %d, got %d\n",
                result.id, wu.id, result.workunitid
            );
            goto done;
        }

        // ****** HERE WE'VE COMMITTED TO SENDING THIS RESULT TO HOST ******
        //

        retval = add_result_to_reply(
            result, wu, sreq, reply, platform, app, avp
        );
        if (!retval) goto done;

dont_send:
        // here we couldn't send the result for some reason --
        // set its state back to PRESENT
        //
        wu_result.state = WR_STATE_PRESENT;
done:
        lock_sema();
    }
    unlock_sema();
}

int send_work(
    SCHEDULER_REQUEST& sreq, SCHEDULER_REPLY& reply, PLATFORM& platform,
    SCHED_SHMEM& ss
) {
// ROMW: Reverting back to older implementation until all clients are 4.x
//       or higher.
#if 1
    reply.wreq.disk_available = max_allowable_disk(sreq);
#else
    reply.wreq.disk_available = sreq.project_disk_free;
#endif
    reply.wreq.insufficient_disk = false;
    reply.wreq.insufficient_mem = false;
    reply.wreq.insufficient_speed = false;
    reply.wreq.no_app_version = false;
    reply.wreq.homogeneous_redundancy_reject = false;
    reply.wreq.daily_result_quota_exceeded = false;
    reply.wreq.core_client_version = sreq.core_client_major_version*100
        + sreq.core_client_minor_version;
    reply.wreq.nresults = 0;

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL,
        "[HOST#%d] got request for %f seconds of work; available disk %f GB\n",
        reply.host.id, sreq.work_req_seconds, reply.wreq.disk_available/1e9
    );

    if (sreq.work_req_seconds <= 0) return 0;

    reply.wreq.seconds_to_fill = sreq.work_req_seconds;
    if (reply.wreq.seconds_to_fill > MAX_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MAX_SECONDS_TO_SEND;
    }
    if (reply.wreq.seconds_to_fill < MIN_SECONDS_TO_SEND) {
        reply.wreq.seconds_to_fill = MIN_SECONDS_TO_SEND;
    }

    if (config.locality_scheduling) {
        reply.wreq.infeasible_only = false;
        send_work_locality(sreq, reply, platform, ss);
    } else {
        // give priority to results that were infeasible for some other host
        //
        reply.wreq.infeasible_only = true;
        scan_work_array(sreq, reply, platform, ss);

        reply.wreq.infeasible_only = false;
        scan_work_array(sreq, reply, platform, ss);
    }

#if 0
    // huh???
    if (wreq.nresults == 0) {
        wreq.disk_available = sreq.potentially_free_offender;
        scan_work_array(wreq, sreq, reply, platform, ss);
    }
    if (wreq.nresults == 0 && config.delete_from_self) {
        wreq.disk_available = sreq.potentially_free_self;
        scan_work_array(wreq, sreq, reply, platform, ss);
    }
#endif

    log_messages.printf(
        SCHED_MSG_LOG::NORMAL, "[HOST#%d] Sent %d results\n",
        reply.host.id, reply.wreq.nresults
    );

    if (reply.wreq.nresults == 0) {
        reply.request_delay = 3600;
        USER_MESSAGE um("No work available", "high");
        reply.insert_message(um);
        if (reply.wreq.no_app_version) {
            USER_MESSAGE um("(there was work for other platforms)", "high");
            reply.insert_message(um);
            reply.request_delay = 3600*24;
        }
        if (reply.wreq.insufficient_disk) {
            USER_MESSAGE um(
                "(there was work but you don't have enough disk space allocated)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.insufficient_mem) {
            USER_MESSAGE um(
                "(there was work but your computer doesn't have enough memory)",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.insufficient_speed) {
            USER_MESSAGE um(
                "(there was work but your computer would not finish it before it is due",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.homogeneous_redundancy_reject) {
            USER_MESSAGE um(
                "(there was work but it was committed to other platforms",
                "high"
            );
            reply.insert_message(um);
        }
        if (reply.wreq.outdated_core) {
            USER_MESSAGE um(
                " (your core client is out of date - please upgrade)",
                "high"
            );
            reply.insert_message(um);
            reply.request_delay = 3600*24;
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "Not sending work because core client is outdated\n"
            );
        }
        if (reply.wreq.daily_result_quota_exceeded) {
            USER_MESSAGE um("(daily quota exceeded)", "high");
            reply.insert_message(um);
            log_messages.printf(
                SCHED_MSG_LOG::NORMAL,
                "Daily result quota exceeded for host %d\n",
                reply.host.id
            );
        }
    }
    return 0;
}

const char *BOINC_RCSID_32dcd335e7 = "$Id$";
