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

// scheduler code related to sending jobs.
// NOTE: there should be nothing here specific to particular
// scheduling policies (array scan, score-based, locality)

#include "config.h"
#include <vector>
#include <list>
#include <string>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "str_util.h"
#include "synch.h"
#include "boinc_stdio.h"

#include "buda.h"
#include "credit.h"
#include "hr.h"
#include "sched_array.h"
#include "sched_assign.h"
#include "sched_config.h"
#include "sched_customize.h"
#include "sched_hr.h"
#include "sched_locality.h"
#include "sched_main.h"
#include "sched_msgs.h"
#include "sched_nci.h"
#include "sched_shmem.h"
#include "sched_score.h"
#include "sched_timezone.h"
#include "sched_types.h"
#include "sched_util.h"
#include "sched_version.h"
#include "sched_send.h"

// if host sends us an impossible RAM size, use this instead
//
const double DEFAULT_RAM_SIZE = 64000000;

int selected_app_message_index=0;

static inline bool file_present_on_host(const char* name) {
    for (unsigned i=0; i<g_request->file_infos.size(); i++) {
        FILE_INFO& fi = g_request->file_infos[i];
        if (!strstr(name, fi.name)) {
            return true;
        }
    }
    return false;
}

// return the number of sticky files present on host, used by job
//
int nfiles_on_host(WORKUNIT& wu) {
    MIOFILE mf;
    mf.init_buf_read(wu.xml_doc);
    XML_PARSER xp(&mf);
    int n=0;
    while (!xp.get_tag()) {
        if (xp.match_tag("file_info")) {
            FILE_INFO fi;
            int retval = fi.parse(xp);
            if (retval) continue;
            if (!fi.sticky) continue;
            if (file_present_on_host(fi.name)) {
                n++;
            }
        }
    }
    return n;
}

// we're going to send the client this job,
// and the app uses locality scheduling lite.
// Add the job's sticky files to the list of files present on host.
//
void add_job_files_to_host(WORKUNIT& wu) {
    MIOFILE mf;
    mf.init_buf_read(wu.xml_doc);
    XML_PARSER xp(&mf);
    while (!xp.get_tag()) {
        if (xp.match_tag("file_info")) {
            FILE_INFO fi;
            int retval = fi.parse(xp);
            if (retval) continue;
            if (!fi.sticky) continue;
            if (!file_present_on_host(fi.name)) {
                if (config.debug_send) {
                    log_messages.printf(MSG_NORMAL,
                        "[send] Adding file %s to host file list\n", fi.name
                    );
                }
                g_request->file_infos.push_back(fi);
            }
        }
    }
}

const double MIN_REQ_SECS = 0;
const double MAX_REQ_SECS = (28*SECONDS_IN_DAY);

// compute effective_ncpus;
// get limits on:
// # jobs per day
// # jobs per RPC
// # jobs in progress
//
void WORK_REQ::get_job_limits() {
    int ninstances[NPROC_TYPES];
    int i;

    memset(ninstances, 0, sizeof(ninstances));
    int n;
    n = g_reply->host.p_ncpus;
    if (g_request->global_prefs.max_ncpus_pct && g_request->global_prefs.max_ncpus_pct < 100) {
        n = (int)((n*g_request->global_prefs.max_ncpus_pct)/100.);
    }
    if (n > config.max_ncpus) n = config.max_ncpus;
    if (n < 1) n = 1;
    if (n > MAX_CPUS) n = MAX_CPUS;
    if (project_prefs.max_cpus) {
        if (n > project_prefs.max_cpus) {
            n = project_prefs.max_cpus;
        }
    }
    ninstances[PROC_TYPE_CPU] = n;
    effective_ncpus = n;

    effective_ngpus = 0;
    for (i=1; i<g_request->coprocs.n_rsc; i++) {
        COPROC& cp = g_request->coprocs.coprocs[i];
        int proc_type = coproc_type_name_to_num(cp.type);
        if (proc_type < 0) continue;
        n = cp.count;
        if (n > MAX_GPUS) n = MAX_GPUS;
        ninstances[proc_type] = n;
        effective_ngpus += n;
    }

    int mult = effective_ncpus + config.gpu_multiplier * effective_ngpus;
    if (config.non_cpu_intensive) {
        mult = 1;
        ninstances[0] = 1;
        if (effective_ngpus) effective_ngpus = 1;
    }

    if (config.max_wus_to_send) {
        g_wreq->max_jobs_per_rpc = mult * config.max_wus_to_send;
    } else {
        g_wreq->max_jobs_per_rpc = 999999;
    }

    if (config.debug_quota) {
        log_messages.printf(MSG_NORMAL,
            "[quota] effective ncpus %d ngpus %d\n",
            effective_ncpus, effective_ngpus
        );
    }
    config.max_jobs_in_progress.reset(ninstances);
}

const char* find_user_friendly_name(int appid) {
    APP* app = ssp->lookup_app(appid);
    if (app) return app->user_friendly_name;
    return "deprecated application";
}

// Called at start of request handling.
// 1) if there's a global jobs/day limit, enforce it using HAV limit
// 2) if last RPC was yesterday or earlier, clear n_jobs_today for HAV
//
static void update_quota(DB_HOST_APP_VERSION& hav) {
    if (config.daily_result_quota) {
        if (hav.max_jobs_per_day == 0) {
            hav.max_jobs_per_day = config.daily_result_quota;
            if (config.debug_quota) {
                log_messages.printf(MSG_NORMAL,
                    "[quota] [HAV#%lu] Initializing max_results_day to %d\n",
                    hav.app_version_id,
                    config.daily_result_quota
                );
            }
        }
    }

    if (g_request->last_rpc_dayofyear != g_request->current_rpc_dayofyear) {
        if (config.debug_quota) {
            log_messages.printf(MSG_NORMAL,
                "[quota] [HOST#%lu] [HAV#%lu] Resetting n_jobs_today\n",
                g_reply->host.id, hav.app_version_id
            );
        }
        hav.n_jobs_today = 0;
    }
}

// see how much RAM we can use on this machine
//
static inline void get_mem_sizes() {
    g_wreq->ram = g_reply->host.m_nbytes;
    if (g_wreq->ram <= 0) g_wreq->ram = DEFAULT_RAM_SIZE;
    g_wreq->usable_ram = g_wreq->ram;
    double busy_frac = g_request->global_prefs.ram_max_used_busy_frac;
    double idle_frac = g_request->global_prefs.ram_max_used_idle_frac;
    double frac = 1;
    if (busy_frac>0 && idle_frac>0) {
        frac = std::max(busy_frac, idle_frac);
        if (frac > 1) frac = 1;
        g_wreq->usable_ram *= frac;
    }
}

// Decide whether or not this app version is 'reliable'
// An app version is reliable if the following conditions are true
// (for those that are set in the config file)
// 1) The host average turnaround is less than a threshold
// 2) consecutive_valid is above a threshold
// 3) The host results per day is equal to the max value
//
void get_reliability_version(HOST_APP_VERSION& hav, double multiplier) {
    if (hav.turnaround.n > MIN_HOST_SAMPLES && config.reliable_max_avg_turnaround) {

        if (hav.turnaround.get_avg() > config.reliable_max_avg_turnaround*multiplier) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [AV#%lu] not reliable; avg turnaround: %.3f > %.3f hrs\n",
                    hav.app_version_id,
                    hav.turnaround.get_avg()/3600,
                    config.reliable_max_avg_turnaround*multiplier/3600
                );
            }
            hav.reliable = false;
            return;
        }
    }
    if (hav.consecutive_valid < CONS_VALID_RELIABLE) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [AV#%lu] not reliable; cons valid %d < %d\n",
                hav.app_version_id,
                hav.consecutive_valid, CONS_VALID_RELIABLE
            );
        }
        hav.reliable = false;
        return;
    }
    if (config.daily_result_quota) {
        if (hav.max_jobs_per_day < config.daily_result_quota) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [AV#%lu] not reliable; max_jobs_per_day %d<%d\n",
                    hav.app_version_id,
                    hav.max_jobs_per_day,
                    config.daily_result_quota
                );
            }
            hav.reliable = false;
            return;
        }
    }
    hav.reliable = true;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] [HOST#%lu] app version %lu is reliable\n",
            g_reply->host.id, hav.app_version_id
        );
    }
    g_wreq->has_reliable_version = true;
}

// decide whether do unreplicated jobs with this app version
//
static void set_trust(DB_HOST_APP_VERSION& hav) {
    hav.trusted = false;
    if (hav.consecutive_valid < CONS_VALID_UNREPLICATED) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] set_trust: cons valid %d < %d, don't use single replication\n",
                hav.consecutive_valid, CONS_VALID_UNREPLICATED
            );
        }
        return;
    }
    double x = 1./hav.consecutive_valid;
    if (drand() > x) hav.trusted = true;
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] set_trust: random choice for cons valid %d: %s\n",
            hav.consecutive_valid, hav.trusted?"yes":"no"
        );
    }
}

static void get_reliability_and_trust() {
    // Platforms other than Windows, Linux and Intel Macs need a
    // larger set of computers to be marked reliable
    //
    double multiplier = 1.0;
    if (strstr(g_reply->host.os_name,"Windows")
        || strstr(g_reply->host.os_name,"Linux")
        || (strstr(g_reply->host.os_name,"Darwin")
            && !(strstr(g_reply->host.p_vendor,"Power Macintosh"))
    )) {
        multiplier = 1.0;
    } else {
        multiplier = 1.8;
    }

    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        get_reliability_version(hav, multiplier);
        set_trust(hav);
    }
}

// Compute the max additional disk usage we can impose on the host.
// Depending on the client version, it can either send us
// - d_total and d_free (pre 4 oct 2005)
// - the above plus d_boinc_used_total and d_boinc_used_project
//
double max_allowable_disk() {
    HOST host = g_request->host;
    GLOBAL_PREFS prefs = g_request->global_prefs;
    double x, x1=0, x2=0, x3;

    // defaults are from config.xml
    // if not there these are used:
    // default_disk_min_free_gb = 1
    //
    if (prefs.disk_min_free_gb < config.default_disk_min_free_gb) {
       prefs.disk_min_free_gb = config.default_disk_min_free_gb;
    }

    // no defaults for total/free disk space (host.d_total, d_free)
    // if they're zero, client will get no work.
    //

    if (host.d_boinc_used_total) {
        // The post 4 oct 2005 case.
        // Compute the max allowable additional disk usage based on prefs
        //
        x3 = host.d_free - prefs.disk_min_free_gb*GIGA;
            // may be negative
        x = x3;
        int which = 3;
        if (prefs.disk_max_used_pct > 0) {
            x2 = host.d_total * prefs.disk_max_used_pct / 100.0
                - host.d_boinc_used_total;
            if (x2 < x) {
                x = x2;
                which = 2;
            }
        }
        if (prefs.disk_max_used_gb > 0) {
            x1 = prefs.disk_max_used_gb*GIGA - host.d_boinc_used_total;
            if (x1 < x) {
                x = x1;
                which = 1;
            }
        }

        // see which bound is the most stringent
        // (for client notification in sched_locality.cpp)
        //
        if (which == 1) {
            g_reply->disk_limits.max_used = x;
        } else if (which == 2) {
            g_reply->disk_limits.max_frac = x;
        } else {
            g_reply->disk_limits.min_free = x;
        }
    } else {
        // here we don't know how much space BOINC is using.
        // so we're kinda screwed.
        // All we can do is assume that BOINC is using zero space.
        // We can't honor the max_used for max_used_pct preferences.
        // We can only honor the min_free pref.
        //
        x = host.d_free - prefs.disk_min_free_gb*GIGA;      // may be negative
        g_reply->disk_limits.min_free = x;
        x1 = x2 = x3 = 0;
    }

    if (x < 0) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: disk_max_used_gb %.2fGB disk_max_used_pct %.2f disk_min_free_gb %.2fGB\n",
                prefs.disk_max_used_gb,
                prefs.disk_max_used_pct,
                prefs.disk_min_free_gb
            );
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: host.d_total %.2fGB host.d_free %.2fGB host.d_boinc_used_total %.2fGB\n",
                host.d_total/GIGA,
                host.d_free/GIGA,
                host.d_boinc_used_total/GIGA
            );
            log_messages.printf(MSG_NORMAL,
                "[send] No disk space available: x1 %.2fGB x2 %.2fGB x3 %.2fGB x %.2fGB\n",
                x1/GIGA, x2/GIGA, x3/GIGA, x/GIGA
            );
        }
        g_wreq->disk.set_insufficient(-x);
        x = 0;
    }
    return x;
}

static double estimate_duration_unscaled(WORKUNIT& wu, BEST_APP_VERSION& bav) {
    double rsc_fpops_est = wu.rsc_fpops_est;
    if (rsc_fpops_est <= 0) rsc_fpops_est = 1e12;
    return rsc_fpops_est/bav.host_usage.projected_flops;
}

// Compute cpu_available_frac and gpu_available_frac.
// These are based on client-supplied data, so do sanity checks
//
#define FRAC_MIN 0.1
static inline void clamp_frac(double& frac, const char* name) {
    if (frac > 1) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] %s=%f; setting to 1\n", name, frac
            );
        }
        frac = 1;
    } else if (frac < FRAC_MIN) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] %s=%f; setting to %f\n", name, frac, FRAC_MIN
            );
        }
        frac = FRAC_MIN;
    }
}

static inline void get_available_fracs() {
    if (g_request->core_client_version<=41900) {
        g_wreq->cpu_available_frac = g_reply->host.on_frac;
        g_wreq->gpu_available_frac = g_reply->host.on_frac; // irrelevant
    } else {
        g_wreq->cpu_available_frac = g_reply->host.active_frac * g_reply->host.on_frac;
        g_wreq->gpu_available_frac = g_reply->host.gpu_active_frac * g_reply->host.on_frac;
    }
    clamp_frac(g_wreq->cpu_available_frac, "CPU available fraction");
    clamp_frac(g_wreq->gpu_available_frac, "GPU available fraction");
}

double available_frac(BEST_APP_VERSION& bav) {
    if (bav.host_usage.uses_gpu()) {
        return g_wreq->gpu_available_frac;
    } else {
        return g_wreq->cpu_available_frac;
    }
}

// estimate the amount of real time to complete this WU,
// taking into account active_frac etc.
// Note: don't factor in resource_share_fraction.
// The core client doesn't necessarily round-robin across all projects.
//
double estimate_duration(WORKUNIT& wu, BEST_APP_VERSION& bav) {
    double edu = estimate_duration_unscaled(wu, bav);
    double ed = edu/available_frac(bav);
    if (config.debug_send_job) {
        log_messages.printf(MSG_NORMAL,
            "[send_job] est. duration for WU %lu: unscaled %.2f scaled %.2f\n",
            wu.id, edu, ed
        );
    }
    return ed;
}

void update_n_jobs_today() {
    for (unsigned int i=0; i<g_wreq->host_app_versions.size(); i++) {
        DB_HOST_APP_VERSION& hav = g_wreq->host_app_versions[i];
        update_quota(hav);
    }
}

static inline void update_estimated_delay(BEST_APP_VERSION& bav, double dt) {
    int pt = bav.host_usage.proc_type;
    if (pt == PROC_TYPE_CPU) {
        g_request->cpu_estimated_delay += dt*bav.host_usage.avg_ncpus/g_request->host.p_ncpus;
    } else {
        COPROC* cp = g_request->coprocs.proc_type_to_coproc(pt);
        cp->estimated_delay += dt*bav.host_usage.gpu_usage/cp->count;
    }
}

// insert "text" right after "after" in the given buffer
//
static int insert_after(char* buffer, const char* after, const char* text) {
    char* p;
    char temp[BLOB_SIZE];

    if (strlen(buffer) + strlen(text) >= BLOB_SIZE-1) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: overflow: %d %d\n",
            (int)strlen(buffer),
            (int)strlen(text)
        );
        return ERR_BUFFER_OVERFLOW;
    }
    p = strstr(buffer, after);
    if (!p) {
        log_messages.printf(MSG_CRITICAL,
            "insert_after: %s not found in %s\n", after, buffer
        );
        return ERR_XML_PARSE;
    }
    p += strlen(after);
    // coverity[fixed_size_dest]
    strcpy(temp, p);
    strcpy(p, text);
    strcat(p, temp);
    return 0;
}

// add elements to WU's xml_doc,
// in preparation for sending it to a client
//
static int insert_wu_tags(WORKUNIT& wu, APP& app) {
    char buf[BLOB_SIZE];

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
    if (!empty(wu.keywords)) {
        char buf2[1024];
        sprintf(buf2,
            "    <job_keyword_ids>%s</job_keyword_ids>\n",
            wu.keywords
        );
        strcat(buf, buf2);
        if (config.debug_keyword) {
            log_messages.printf(MSG_NORMAL,
                "[keyword] keywords: %s\n", wu.keywords
            );
        }
    }
    return insert_after(wu.xml_doc, "<workunit>\n", buf);
}

// add host usage into to WU's xml_doc (for BUDA jobs)
//
static int add_usage_to_wu(WORKUNIT &wu, HOST_USAGE &hu) {
    char buf[2048], buf2[2048];
    snprintf(buf, sizeof(buf),
        "   <avg_ncpus>%f</avg_ncpus>\n"
        "   <flops>%f</flops>\n",
        hu.avg_ncpus,
        hu.projected_flops
    );
    if (hu.proc_type != PROC_TYPE_CPU) {
        snprintf(buf2, sizeof(buf2),
            "   <coproc>\n"
            "        <type>%s</type>\n"
            "        <count>%f</count>\n"
            "    </coproc>\n",
            proc_type_name_xml(hu.proc_type),
            hu.gpu_usage
        );
        strcat(buf, buf2);
    }
    if (strlen(hu.cmdline)) {
        snprintf(buf2, sizeof(buf2),
            "   <cmdline>%s</cmdline>\n",
            hu.cmdline
        );
        strcat(buf, buf2);
    }

    char *p = wu.xml_doc;
    if (strlen(p) + strlen(buf) + 10 > sizeof(wu.xml_doc)) {
        log_messages.printf(MSG_CRITICAL,
            "add_usage_to_wu(): field too small: %ld %ld %ld\n",
            strlen(p), strlen(buf), sizeof(wu.xml_doc)
        );
        return -1;
    }
    p = strstr(p, "</workunit>");
    if (!p) {
        log_messages.printf(MSG_CRITICAL, "add_usage_to_wu(): no end tag\n");
        return -1;
    }
    strcpy(p, buf);
    strcat(p, "</workunit>");
    return 0;
}

// Add the given workunit, app, and app version to a reply.
//
static int add_wu_to_reply(
    WORKUNIT& wu, SCHEDULER_REPLY&, APP* app, BEST_APP_VERSION* bavp,
    BUDA_VARIANT *bvp, HOST_USAGE &hu
) {
    int retval;
    WORKUNIT wu2, wu3;

    APP_VERSION* avp = bavp->avp;

    // add the app, app_version, and workunit to the reply,
    // but only if they aren't already there
    //
    if (avp) {
        APP_VERSION av2=*avp, *avp2=&av2;

        if (strlen(config.replace_download_url_by_timezone)) {
            process_av_timezone(avp, av2);
        }

        g_reply->insert_app_unique(*app);
        av2.bavp = bavp;
        g_reply->insert_app_version_unique(*avp2);
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] Sending app_version %s %lu %d %s; projected %.2f GFLOPS\n",
                app->name,
                avp2->platformid, avp2->version_num, avp2->plan_class,
                bavp->host_usage.projected_flops/1e9
            );
        }
    }

    // modify the WU's xml_doc; add <name>, <rsc_*> etc.
    //
    wu2 = wu;       // make copy since we're going to modify its XML field

    // check if plan class specified memory usage
    //
    if (bavp->host_usage.mem_usage) {
        wu2.rsc_memory_bound = bavp->host_usage.mem_usage;
    }

    // adjust FPOPS figures for anonymous platform
    //
    if (bavp->cavp) {
        wu2.rsc_fpops_est *= bavp->cavp->rsc_fpops_scale;
        wu2.rsc_fpops_bound *= bavp->cavp->rsc_fpops_scale;
    }
    retval = insert_wu_tags(wu2, *app);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "insert_wu_tags failed: %s\n", boincerror(retval)
        );
        return retval;
    }

    if (bvp) {
        retval = add_usage_to_wu(wu2, hu);
        if (retval) return retval;
        add_app_files(wu2, *bvp);
    }
    wu3 = wu2;
    if (strlen(config.replace_download_url_by_timezone)) {
        process_wu_timezone(wu2, wu3);
    }

    g_reply->insert_workunit_unique(wu3);

    return 0;
}

// add <name> tags to result's xml_doc_in
//
static int insert_name_tags(RESULT& result, WORKUNIT const& wu) {
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

static int insert_deadline_tag(RESULT& result) {
    char buf[256];
    sprintf(buf, "<report_deadline>%d</report_deadline>\n", result.report_deadline);
    int retval = insert_after(result.xml_doc_in, "<result>\n", buf);
    if (retval) return retval;
    return 0;
}

// update workunit fields when send an instance of it:
// - transition time
// - app_version_id, if app uses homogeneous app version
// - hr_class, if we're using HR
//
// In the latter two cases, the update is conditional on the field
// fields either being zero or the desired value.
// Some other scheduler instance might have updated it since we read the WU,
// and the transitioner might have set it to zero.
//
int update_wu_on_send(WORKUNIT wu, time_t x, APP& app, BEST_APP_VERSION& bav) {
    DB_WORKUNIT dbwu;
    char buf[256], buf2[256], where_clause[256];
    int retval;

    dbwu.id = wu.id;

    // SQL note: can't use min() here
    //
    sprintf(buf,
        "transition_time=if(transition_time<%d, transition_time, %d)",
        (int)x, (int)x
    );
    strcpy(where_clause, "");
    if (app.homogeneous_app_version) {
        sprintf(buf2, ", app_version_id=%lu", bav.avp->id);
        strcat(buf, buf2);
        sprintf(where_clause,
            "(app_version_id=0 or app_version_id=%lu)", bav.avp->id
        );
    }
    if (app_hr_type(app)) {
        int host_hr_class = hr_class(g_request->host, app_hr_type(app));
        sprintf(buf2, ", hr_class=%d", host_hr_class);
        strcat(buf, buf2);
        if (strlen(where_clause)) {
            strcat(where_clause, " and ");
        }
        sprintf(buf2, "(hr_class=0 or hr_class=%d)", host_hr_class);
        strcat(where_clause, buf2);
    }
    retval = dbwu.update_field(buf, strlen(where_clause)?where_clause:NULL);
    if (retval) return retval;
    if (boinc_db.affected_rows() != 1) {
        return ERR_DB_NOT_FOUND;
    }
    return 0;
}

// return true iff a result for same WU is already being sent
//
bool wu_already_in_reply(WORKUNIT& wu) {
    unsigned int i;
    for (i=0; i<g_reply->results.size(); i++) {
        if (wu.id == g_reply->results[i].workunitid) {
            return true;
        }
    }
    return false;
}

void lock_sema() {
    lock_semaphore(sema_key);
}

void unlock_sema() {
    unlock_semaphore(sema_key);
}

static inline bool have_apps(int pt) {
    if (g_wreq->anonymous_platform) {
        return g_wreq->client_has_apps_for_proc_type[pt];
    } else {
        return ssp->have_apps_for_proc_type[pt];
    }
}

// return true if additional work is needed,
// and there's disk space left,
// and we haven't exceeded result per RPC limit,
// and we haven't exceeded results per day limit
//
bool work_needed(bool locality_sched) {
    if (locality_sched) {
        // if we've failed to send a result because of a transient condition,
        // return false to preserve invariant
        //
        if (g_wreq->disk.insufficient) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] stopping work search - insufficient disk space\n"
                );
            }
            return false;
        }
        if (g_wreq->speed.insufficient) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] stopping work search - host too slow\n"
                );
            }
            return false;
        }
        if (g_wreq->mem.insufficient) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] stopping work search - insufficient memory\n"
                );
            }
            return false;
        }
        if (g_wreq->no_allowed_apps_available) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] stopping work search - no locality app selected\n"
                );
            }
            return false;
        }
    }

    // check user-specified project prefs limit on # of jobs in progress
    //
    int mj = g_wreq->project_prefs.max_jobs_in_progress;
    if (mj && config.max_jobs_in_progress.project_limits.total.njobs >= mj) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] user project preferences job limit exceeded\n"
            );
        }
        g_wreq->max_jobs_on_host_exceeded = true;
        return false;
    }

    // check config.xml limits on in-progress jobs
    //
    bool some_type_allowed = false;

    for (int i=0; i<NPROC_TYPES; i++) {
        if (!have_apps(i)) continue;
        if (config.max_jobs_in_progress.exceeded(NULL, i)) {
            if (config.debug_quota) {
                log_messages.printf(MSG_NORMAL,
                    "[quota] reached limit on %s jobs in progress\n",
                    proc_type_name(i)
                );
                config.max_jobs_in_progress.print_log();
            }
            g_wreq->clear_req(i);
            g_wreq->max_jobs_on_host_proc_type_exceeded[i] = true;
        } else {
            some_type_allowed = true;
        }
    }

    if (!some_type_allowed) {
        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] config.xml max_jobs_in_progress limit exceeded\n"
            );
        }
        g_wreq->max_jobs_on_host_exceeded = true;
        return false;
    }

    // see if we've reached max jobs per RPC
    //
    if (g_wreq->njobs_sent >= g_wreq->max_jobs_per_rpc) {
        if (config.debug_quota) {
            log_messages.printf(MSG_NORMAL,
                "[quota] stopping work search - njobs %d >= max_jobs_per_rpc %d\n",
                g_wreq->njobs_sent, g_wreq->max_jobs_per_rpc
            );
        }
        return false;
    }

#if 0
    if (config.debug_send) {
        char buf[256], buf2[256];
        strcpy(buf, "");
        for (int i=0; i<NPROC_TYPES; i++) {
            sprintf(buf2, " %s (%.2f, %.2f)",
                proc_type_name(i),
                g_wreq->req_secs[i],
                g_wreq->req_instances[i]
            );
            strcat(buf, buf2);
        }
        log_messages.printf(MSG_NORMAL,
            "[send] work_needed: spec req %d sec to fill %.2f; %s\n",
            g_wreq->rsc_spec_request,
            g_wreq->seconds_to_fill,
            buf
        );
    }
#endif
    if (g_wreq->rsc_spec_request) {
        for (int i=0; i<NPROC_TYPES; i++) {
            if (g_wreq->need_proc_type(i) && have_apps(i)) {
                return true;
            }
        }
    } else {
        if (g_wreq->seconds_to_fill > 0) {
            return true;
        }
    }
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL, "[send] don't need more work\n");
    }
    return false;
}

// return the app version ID, or -2/-3/-4 if anonymous platform
//
inline static DB_ID_TYPE get_app_version_id(BEST_APP_VERSION* bavp) {
    if (bavp->avp) {
        return bavp->avp->id;
    } else {
        return bavp->cavp->host_usage.resource_type();
    }
}

static bool wu_has_plan_class(WORKUNIT &wu, char* buf) {
    char *p = strstr(wu.xml_doc, "<plan_class>");
    if (!p) return false;
    p += strlen("<plan_class>");
    strncpy(buf, p, 256);
    p = strstr(buf, "</plan_class>");
    if (!p) return false;
    *p = 0;
    return true;
}

int add_result_to_reply(
    SCHED_DB_RESULT& result,
    WORKUNIT& wu,
    BEST_APP_VERSION* bavp,
    HOST_USAGE &host_usage,
    BUDA_VARIANT *bvp,
    bool locality_scheduling
) {
    int retval;
    bool resent_result = false;
    APP* app = ssp->lookup_app(wu.appid);

    result.hostid = g_reply->host.id;
    result.userid = g_reply->user.id;
    result.sent_time = time(0);
    result.report_deadline = result.sent_time + wu.delay_bound;
    result.flops_estimate = host_usage.peak_flops;
    result.app_version_id = get_app_version_id(bavp);

    // update WU DB record.
    // This can fail in normal operation
    // (other scheduler already updated hr_class or app_version_id)
    // so do it before updating the result.
    //
    retval = update_wu_on_send(
        wu, result.report_deadline + config.report_grace_period, *app, *bavp
    );
    if (retval == ERR_DB_NOT_FOUND) {
        log_messages.printf(MSG_NORMAL,
            "add_result_to_reply: WU already sent to other HR class or app version\n"
        );
        return retval;
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: WU update failed: %d\n",
            retval
        );
        return retval;
    }

    // update result DB record.
    // This can also fail in normal operation.
    // In this case, in principle we should undo
    // the changes we just made to the WU (or use a transaction)
    // but I don't think it actually matters.
    //
    int old_server_state = result.server_state;

    if (result.server_state != RESULT_SERVER_STATE_IN_PROGRESS) {
        // We're sending this result for the first time
        //
        result.server_state = RESULT_SERVER_STATE_IN_PROGRESS;
    } else {
        // Result was already sent to this host but was lost,
        // so we're resending it.
        //
        resent_result = true;

        if (config.debug_send) {
            log_messages.printf(MSG_NORMAL,
                "[send] [RESULT#%lu] [HOST#%lu] (resend lost work)\n",
                result.id, g_reply->host.id
            );
        }
    }
    retval = result.mark_as_sent(old_server_state, config.report_grace_period);
    if (retval == ERR_DB_NOT_FOUND) {
        log_messages.printf(MSG_CRITICAL,
            "[RESULT#%lu] [HOST#%lu]: CAN'T SEND, already sent to another host\n",
            result.id, g_reply->host.id
        );
    } else if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't update result: %s\n", boincerror(retval)
        );
    }
    if (retval) return retval;

    // done with DB updates.
    //

    retval = add_wu_to_reply(wu, *g_reply, app, bavp, bvp, host_usage);
    if (retval) return retval;

    // Adjust available disk space.
    // In the locality scheduling locality case,
    // reduce the available space by less than the workunit rsc_disk_bound,
    // if the host already has the file or the file was not already sent.
    //
    if (!locality_scheduling || decrement_disk_space_locality(wu)) {
        g_wreq->disk_available -= wu.rsc_disk_bound;
    }

    double est_dur = estimate_duration(wu, *bavp);
    if (config.debug_send) {
        double max_time = wu.rsc_fpops_bound / host_usage.projected_flops;
        char buf1[64],buf2[64];
        secs_to_hmsf(est_dur, buf1);
        secs_to_hmsf(max_time, buf2);
        log_messages.printf(MSG_NORMAL,
            "[send] [HOST#%lu] sending [RESULT#%lu %s] (est. dur. %.2fs (%s)) (max time %.2fs (%s))\n",
            g_reply->host.id, result.id, result.name, est_dur, buf1, max_time, buf2
        );
    }

    // The following overwrites the result's xml_doc field.
    // But that's OK cuz we're done with DB updates
    //
    retval = insert_name_tags(result, wu);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert name tags: %d\n",
            retval
        );
        return retval;
    }
    retval = insert_deadline_tag(result);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "add_result_to_reply: can't insert deadline tag: %s\n", boincerror(retval)
        );
        return retval;
    }
    result.bav = *bavp;
    g_reply->insert_result(result);

    // decrement the work requests (seconds and instances)
    // based on the estimated duration of this job
    // and how many instances it uses.
    //
    // If it's a GPU job, don't decrement the CPU requests,
    // because the scheduling of GPU jobs is constrained by the # of GPUs
    //
    if (g_wreq->rsc_spec_request) {
        int pt = host_usage.proc_type;
        if (pt == PROC_TYPE_CPU) {
            double est_cpu_secs = est_dur*host_usage.avg_ncpus;
            g_wreq->req_secs[PROC_TYPE_CPU] -= est_cpu_secs;
            g_wreq->req_instances[PROC_TYPE_CPU] -= host_usage.avg_ncpus;
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] est_dur %f est_cpu_secs %f; new req_secs %f\n",
                    est_dur, est_cpu_secs, g_wreq->req_secs[PROC_TYPE_CPU]
                );
            }
        } else {
            double est_gpu_secs = est_dur*host_usage.gpu_usage;
            g_wreq->req_secs[pt] -= est_gpu_secs;
            g_wreq->req_instances[pt] -= host_usage.gpu_usage;
            if (config.debug_send_job) {
                log_messages.printf(MSG_NORMAL,
                    "[send_job] est_dur %f est_gpu_secs %f; new req_secs %f\n",
                    est_dur, est_gpu_secs, g_wreq->req_secs[pt]
                );
            }
        }
    } else {
        // extremely old clients don't send per-resource requests
        g_wreq->seconds_to_fill -= est_dur;
    }
    update_estimated_delay(*bavp, est_dur);
    g_wreq->njobs_sent++;
    config.max_jobs_in_progress.register_job(app, host_usage.proc_type);
    if (!resent_result) {
        DB_HOST_APP_VERSION* havp = bavp->host_app_version();
        if (havp) {
            havp->n_jobs_today++;
        }
    }

    // add this result to workload for simulation
    //
    if (config.workload_sim && g_request->have_other_results_list) {
        IP_RESULT ipr ("", time(0)+wu.delay_bound, est_dur);
        g_request->ip_results.push_back(ipr);
    }

    // mark job as done if debugging flag is set;
    // this is used by sched_driver.C (performance testing)
    //
    if (mark_jobs_done) {
        DB_WORKUNIT dbwu;
        char buf[256];
        sprintf(buf,
            "server_state=%d outcome=%d",
            RESULT_SERVER_STATE_OVER, RESULT_OUTCOME_SUCCESS
        );
        result.update_field(buf);

        dbwu.id = wu.id;
        sprintf(buf, "transition_time=%ld", time(0));
        dbwu.update_field(buf);

    }

    // If we're sending an unreplicated job to an untrusted host,
    // mark it as replicated
    //
    if (wu.target_nresults == 1 && app->target_nresults > 1) {
        if (bavp->trusted) {
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%lu] using trusted app version, not replicating\n", wu.id
                );
            }
        } else {
            DB_WORKUNIT dbwu;
            char buf[256];
            sprintf(buf,
                "target_nresults=%d, min_quorum=%d, transition_time=%ld",
                app->target_nresults, app->target_nresults, time(0)
            );
            dbwu.id = wu.id;
            if (config.debug_send) {
                log_messages.printf(MSG_NORMAL,
                    "[send] [WU#%lu] sending to untrusted host, replicating\n", wu.id
                );
            }
            retval = dbwu.update_field(buf);
            if (retval) {
                log_messages.printf(MSG_CRITICAL,
                    "WU update failed: %s", boincerror(retval)
                );
            }
        }
    }

    // if the app uses locality scheduling lite,
    // add the job's files to the list of those on host
    //
    if (app->locality_scheduling == LOCALITY_SCHED_LITE) {
        add_job_files_to_host(wu);
    }

    return 0;
}

// Send high-priority messages about things the user can change easily
// (namely the driver version)
// and low-priority messages about things that can't easily be changed,
// but which may be interfering with getting tasks or latest apps
//
static void send_gpu_property_messages(
    GPU_REQUIREMENTS& req, double ram, int version, const char* rsc_name
) {
    char buf[256];
    if (ram < req.min_ram) {
        sprintf(buf,
            "A minimum of %d MB (preferably %d MB) of video RAM is needed to process tasks using your computer's %s",
            (int) (req.min_ram/MEGA),
            (int) (req.opt_ram/MEGA),
            rsc_name
        );
        g_reply->insert_message(buf, "low");
    } else {
        if (version) {
            if (version < req.min_driver_version) {
                sprintf(buf,
                    "%s: %s",
                    rsc_name,
                    _("Upgrade to the latest driver to process tasks using your computer's GPU")
                );
                g_reply->insert_message(buf, "notice");
            } else if (version < req.opt_driver_version) {
                sprintf(buf,
                    "%s: %s",
                    rsc_name,
                    _("Upgrade to the latest driver to use all of this project's GPU applications")
                );
                g_reply->insert_message(buf, "low");
            }
        }
    }
}

// send messages complaining about lack of GPU or the properties of GPUs
//
void send_gpu_messages() {
    // Mac client with GPU but too-old client
    //
    if (g_request->coprocs.nvidia.count
        && ssp->have_apps_for_proc_type[PROC_TYPE_NVIDIA_GPU]
        && strstr(g_request->host.os_name, "Darwin")
        && g_request->core_client_version < 61028
    ) {
        g_reply->insert_message(
            _("A newer version of BOINC is needed to use your NVIDIA GPU; please upgrade to the current version"),
            "notice"
        );
    }

    // GPU-only project, client lacks GPU
    //
    bool usable_gpu = false;
    bool have_gpu_apps = false;
    for (int i=1; i<NPROC_TYPES; i++) {
        if (ssp->have_apps_for_proc_type[i]) {
            have_gpu_apps = true;
            COPROC* cp = g_request->coprocs.proc_type_to_coproc(i);
            if (cp && cp->count) {
                usable_gpu = true;
            }
        }
    }
    if (!ssp->have_apps_for_proc_type[PROC_TYPE_CPU]
        && have_gpu_apps
        && !usable_gpu
    ) {
        char buf[256];
        strcpy(buf, "");
        for (int i=1; i<NPROC_TYPES; i++) {
            if (ssp->have_apps_for_proc_type[i]) {
                if (strlen(buf)) {
                    strcat(buf, " or ");
                }
                strcat(buf, proc_type_name(i));
            }
        }
        char msg[1024];
        sprintf(msg,
            _("An %s GPU is required to run tasks for this project"),
            buf
        );
        g_reply->insert_message(msg, "notice");
    }

    if (g_request->coprocs.nvidia.count && ssp->have_apps_for_proc_type[PROC_TYPE_NVIDIA_GPU]) {
        send_gpu_property_messages(gpu_requirements[PROC_TYPE_NVIDIA_GPU],
            g_request->coprocs.nvidia.prop.totalGlobalMem,
            g_request->coprocs.nvidia.display_driver_version,
            proc_type_name(PROC_TYPE_NVIDIA_GPU)
        );
    }
    if (g_request->coprocs.ati.count && ssp->have_apps_for_proc_type[PROC_TYPE_AMD_GPU]) {
        send_gpu_property_messages(gpu_requirements[PROC_TYPE_AMD_GPU],
            g_request->coprocs.ati.attribs.localRAM*MEGA,
            g_request->coprocs.ati.version_num,
            proc_type_name(PROC_TYPE_AMD_GPU)
        );
    }
    if (g_request->coprocs.intel_gpu.count && ssp->have_apps_for_proc_type[PROC_TYPE_INTEL_GPU]) {
        send_gpu_property_messages(gpu_requirements[PROC_TYPE_INTEL_GPU],
            g_request->coprocs.intel_gpu.opencl_prop.global_mem_size,
            0,
            proc_type_name(PROC_TYPE_INTEL_GPU)
        );
    }
    if (g_request->coprocs.apple_gpu.count && ssp->have_apps_for_proc_type[PROC_TYPE_APPLE_GPU]) {
        send_gpu_property_messages(gpu_requirements[PROC_TYPE_APPLE_GPU],
            g_request->coprocs.apple_gpu.opencl_prop.global_mem_size,
            0,
            proc_type_name(PROC_TYPE_APPLE_GPU)
        );
    }
}

// send messages to user about why jobs were or weren't sent,
// recommendations for GPU driver upgrades, etc.
//
static void send_user_messages() {
    char buf[512];
    unsigned int i;
    int j;

    // GPU messages aren't relevant if anonymous platform
    //
    if (!g_wreq->anonymous_platform) {
        send_gpu_messages();
    }

    // If work was sent from apps the user did not select, explain.
    // NOTE: this will have to be done differently with matchmaker scheduling
    //
    if (!config.locality_scheduling && !config.locality_scheduler_fraction && config.sched_old) {
        if (g_wreq->njobs_sent && !g_wreq->user_apps_only) {
            g_reply->insert_message(
                "No tasks are available for the applications you have selected",
                "low"
            );

            // Inform the user about applications with no work
            //
            for (i=0; i<g_wreq->project_prefs.selected_apps.size(); i++) {
                if (!g_wreq->project_prefs.selected_apps[i].work_available) {
                    APP* app = ssp->lookup_app(g_wreq->project_prefs.selected_apps[i].appid);
                    // don't write message if the app is deprecated
                    //
                    if (app) {
                        char explanation[256];
                        sprintf(explanation,
                            "No tasks are available for %s",
                            find_user_friendly_name(g_wreq->project_prefs.selected_apps[i].appid)
                        );
                        g_reply->insert_message( explanation, "low");
                    }
                }
            }

            // Tell the user about applications they didn't qualify for
            //
            for (j=0; j<selected_app_message_index; j++){
                g_reply->insert_message(g_wreq->no_work_messages.at(j));
            }
            g_reply->insert_message(
                "Your preferences allow tasks from applications other than those selected",
                "low"
            );
            g_reply->insert_message(
                "Sending tasks from other applications", "low"
            );
        }
    }

    // if client asked for work and we're not sending any, explain why
    //
    if (g_wreq->njobs_sent == 0 && g_request->work_req_seconds) {
        g_reply->set_delay(DELAY_NO_WORK_TEMP);
        g_reply->insert_message("No tasks sent", "low");

        // Tell the user about applications with no work
        //
        for (i=0; i<g_wreq->project_prefs.selected_apps.size(); i++) {
            if (!g_wreq->project_prefs.selected_apps[i].work_available) {
                APP* app = ssp->lookup_app(g_wreq->project_prefs.selected_apps[i].appid);
                // don't write message if the app is deprecated
                if (app != NULL) {
                    sprintf(buf, "No tasks are available for %s",
                        find_user_friendly_name(
                            g_wreq->project_prefs.selected_apps[i].appid
                        )
                    );
                    g_reply->insert_message(buf, "low");
                }
            }
        }

        for (i=0; i<g_wreq->no_work_messages.size(); i++){
            g_reply->insert_message(g_wreq->no_work_messages.at(i));
        }

        if (g_wreq->no_allowed_apps_available) {
            g_reply->insert_message(
                _("No tasks are available for the applications you have selected."),
                "low"
            );
        }
        if (g_wreq->speed.insufficient) {
            if (g_request->core_client_version>41900) {
                sprintf(buf,
                    "Tasks won't finish in time: BOINC runs %.1f%% of the time; computation is enabled %.1f%% of that",
                    100*g_reply->host.on_frac, 100*g_reply->host.active_frac
                );
            } else {
                sprintf(buf,
                    "Tasks won't finish in time: Computer available %.1f%% of the time",
                    100*g_reply->host.on_frac
                );
            }
            g_reply->insert_message(buf, "low");
        }
        if (g_wreq->hr_reject_temp) {
            g_reply->insert_message(
                "Tasks are committed to other platforms",
                "low"
            );
        }
        if (g_wreq->hr_reject_perm) {
            g_reply->insert_message(
                _("Your computer type is not supported by this project"),
                "notice"
            );
        }
        if (g_wreq->outdated_client) {
            g_reply->insert_message(
                _("Newer BOINC version required; please install current version"),
                "notice"
            );
            g_reply->set_delay(DELAY_NO_WORK_PERM);
            log_messages.printf(MSG_NORMAL,
                "Not sending tasks because newer client version required\n"
            );
        }
        for (i=0; i<NPROC_TYPES; i++) {
            if (g_wreq->project_prefs.dont_use_proc_type[i] && ssp->have_apps_for_proc_type[i]) {
                sprintf(buf,
                    _("Tasks for %s are available, but your preferences are set to not accept them"),
                    proc_type_name(i)
                );
                g_reply->insert_message(buf, "low");
            }
        }
        DB_HOST_APP_VERSION* havp = quota_exceeded_version();
        if (havp) {
            sprintf(buf, "This computer has finished a daily quota of %d tasks",
                havp->max_jobs_per_day
            );
            g_reply->insert_message(buf, "low");
            if (config.debug_quota) {
                log_messages.printf(MSG_NORMAL,
                    "[quota] Daily quota %d exceeded for app version %lu\n",
                    havp->max_jobs_per_day, havp->app_version_id
                );
            }
            g_reply->set_delay(DELAY_NO_WORK_CACHE);
        }
        if (g_wreq->max_jobs_exceeded()) {
            sprintf(buf, "This computer has reached a limit on tasks in progress");
            g_reply->insert_message(buf, "low");
            g_reply->set_delay(DELAY_NO_WORK_CACHE);
        }
    }
}

static double clamp_req_sec(double x) {
    if (x < MIN_REQ_SECS) return MIN_REQ_SECS;
    if (x > MAX_REQ_SECS) return MAX_REQ_SECS;
    return x;
}

// prepare to send jobs, both resent and new;
// decipher request type, fill in WORK_REQ
//
void send_work_setup() {
    unsigned int i;

    g_wreq->seconds_to_fill = clamp_req_sec(g_request->work_req_seconds);
    g_wreq->req_secs[PROC_TYPE_CPU] = clamp_req_sec(g_request->cpu_req_secs);
    g_wreq->req_instances[PROC_TYPE_CPU] = g_request->cpu_req_instances;
    g_wreq->anonymous_platform = is_anonymous(g_request->platforms.list[0]);

    // decide on attributes of HOST_APP_VERSIONS
    //
    get_reliability_and_trust();

    // parse project preferences (e.g. no GPUs)
    //
    g_wreq->project_prefs.parse();

    if (g_wreq->anonymous_platform) {
        estimate_flops_anon_platform();

        for (i=0; i<NPROC_TYPES; i++) {
            g_wreq->client_has_apps_for_proc_type[i] = false;
        }
        for (i=0; i<g_request->client_app_versions.size(); i++) {
            CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
            int pt = cav.host_usage.proc_type;
            g_wreq->client_has_apps_for_proc_type[pt] = true;
        }
    }
    for (i=1; i<NPROC_TYPES; i++) {
        gpu_requirements[i].clear();
    }

    g_wreq->disk_available = max_allowable_disk();
    get_mem_sizes();
    get_available_fracs();
    g_wreq->get_job_limits();

    // do sanity checking on GPU scheduling parameters
    //
    for (i=1; i<NPROC_TYPES; i++) {
        COPROC* cp = g_request->coprocs.proc_type_to_coproc(i);
        if (cp && cp->count) {
            g_wreq->req_secs[i] = clamp_req_sec(cp->req_secs);
            g_wreq->req_instances[i] = cp->req_instances;
            if (cp->estimated_delay < 0) {
                cp->estimated_delay = g_request->cpu_estimated_delay;
            }
        }
    }
    g_wreq->rsc_spec_request = false;
    for (i=0; i<NPROC_TYPES; i++) {
        if (g_wreq->req_secs[i]) {
            g_wreq->rsc_spec_request = true;
            break;
        }
    }

    for (i=0; i<g_request->other_results.size(); i++) {
        OTHER_RESULT& r = g_request->other_results[i];
        APP* app = NULL;
        int proc_type = PROC_TYPE_CPU;
        bool have_cav = false;
        if (r.app_version >= 0
            && r.app_version < (int)g_request->client_app_versions.size()
        ) {
            CLIENT_APP_VERSION& cav = g_request->client_app_versions[r.app_version];
            app = cav.app;
            if (app) {
                have_cav = true;
                proc_type = cav.host_usage.proc_type;
            }
        }
        if (!have_cav) {
            if (r.have_plan_class) {
                proc_type = plan_class_to_proc_type(r.plan_class);
            }
        }
        config.max_jobs_in_progress.register_job(app, proc_type);
    }

    // print details of request to log
    //
    if (config.debug_quota) {
        log_messages.printf(MSG_NORMAL,
            "[quota] max jobs per RPC: %d\n", g_wreq->max_jobs_per_rpc
        );
        config.max_jobs_in_progress.print_log();
    }
    if (config.debug_send) {
        log_messages.printf(MSG_NORMAL,
            "[send] %s old scheduling; %s EDF sim\n",
            config.sched_old?"Using":"Not using",
            config.workload_sim?"Using":"Not using"
        );
        log_messages.printf(MSG_NORMAL,
            "[send] CPU: req %.2f sec, %.2f instances; est delay %.2f\n",
            g_wreq->req_secs[PROC_TYPE_CPU],
            g_wreq->req_instances[PROC_TYPE_CPU],
            g_request->cpu_estimated_delay
        );
        for (i=1; i<NPROC_TYPES; i++) {
            COPROC* cp = g_request->coprocs.proc_type_to_coproc(i);
            if (cp && cp->count) {
                log_messages.printf(MSG_NORMAL,
                    "[send] %s: req %.2f sec, %.2f instances; est delay %.2f\n",
                    proc_type_name(i),
                    g_wreq->req_secs[i],
                    g_wreq->req_instances[i],
                    cp->estimated_delay
                );
            }
        }
        log_messages.printf(MSG_NORMAL,
            "[send] work_req_seconds: %.2f secs\n",
            g_wreq->seconds_to_fill
        );
        log_messages.printf(MSG_NORMAL,
            "[send] available disk %.2f GB, work_buf_min %d\n",
            g_wreq->disk_available/GIGA,
            (int)g_request->global_prefs.work_buf_min()
        );
        log_messages.printf(MSG_NORMAL,
            "[send] on_frac %f active_frac %f gpu_active_frac %f\n",
            g_reply->host.on_frac,
            g_reply->host.active_frac,
            g_reply->host.gpu_active_frac
        );
        if (g_wreq->anonymous_platform) {
            log_messages.printf(MSG_NORMAL,
                "[send] Anonymous platform app versions:\n"
            );
            for (i=0; i<g_request->client_app_versions.size(); i++) {
                CLIENT_APP_VERSION& cav = g_request->client_app_versions[i];
                char buf[256];
                strcpy(buf, "");
                int pt = cav.host_usage.proc_type;
                if (pt) {
                    sprintf(buf, " %.2f %s GPU",
                        cav.host_usage.gpu_usage,
                        proc_type_name(pt)
                    );
                }

                log_messages.printf(MSG_NORMAL,
                    "   app: %s version %d cpus %.2f%s flops %fG\n",
                    cav.app_name,
                    cav.version_num,
                    cav.host_usage.avg_ncpus,
                    buf,
                    cav.host_usage.projected_flops/1e9
                );
            }
        }
#if 0
        log_messages.printf(MSG_NORMAL,
            "[send] p_vm_extensions_disabled: %s\n",
            g_request->host.p_vm_extensions_disabled?"yes":"no"
        );
#endif
        log_messages.printf(MSG_NORMAL,
            "[send] CPU features: %s\n", g_request->host.p_features
        );
    }
}

// If a record is not in DB, create it.
//
int update_host_app_versions(vector<SCHED_DB_RESULT>& results, int hostid) {
    vector<DB_HOST_APP_VERSION> new_havs;
    unsigned int i, j;
    int retval;

    for (i=0; i<results.size(); i++) {
        RESULT& r = results[i];
        int gavid = generalized_app_version_id(r.app_version_id, r.appid);
        DB_HOST_APP_VERSION* havp = gavid_to_havp(gavid);
        if (!havp) {
            bool found = false;
            for (j=0; j<new_havs.size(); j++) {
                DB_HOST_APP_VERSION& hav = new_havs[j];
                if (hav.app_version_id == gavid) {
                    found = true;
                    hav.n_jobs_today++;
                }
            }
            if (!found) {
                DB_HOST_APP_VERSION hav;
                hav.clear();
                hav.host_id = hostid;
                hav.app_version_id = gavid;
                hav.n_jobs_today = 1;
                new_havs.push_back(hav);
            }
        }
    }

    // create new records
    //
    for (i=0; i<new_havs.size(); i++) {
        DB_HOST_APP_VERSION& hav = new_havs[i];

        retval = hav.insert();
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "hav.insert(): %s\n", boincerror(retval)
            );
        } else {
            if (config.debug_credit) {
                log_messages.printf(MSG_NORMAL,
                    "[credit] created host_app_version record (%lu, %lu)\n",
                    hav.host_id, hav.app_version_id
                );
            }
        }
    }
    return 0;
}

void send_work() {
    int retval;

    if (all_apps_use_hr && hr_unknown_platform(g_request->host)) {
        log_messages.printf(MSG_NORMAL,
            "Not sending work because unknown HR class\n"
        );
        g_wreq->hr_reject_perm = true;
        return;
    }

    if (config.enable_assignment) {
        if (send_targeted_jobs()) {
            if (config.debug_assignment) {
                log_messages.printf(MSG_NORMAL,
                    "[assign] [HOST#%lu] sent assigned jobs\n", g_reply->host.id
                );
            }
            goto done;
        }
    }

    // if user is job submitter and has 'only run jobs on my computers' set,
    // send them only their own jobs
    //
    if (g_reply->user.seti_id) {
        goto done;
    }

    if (config.enable_assignment_multi) {
        if (send_broadcast_jobs()) {
            if (config.debug_assignment) {
                log_messages.printf(MSG_NORMAL,
                    "[assign] [HOST#%lu] sent assigned jobs\n", g_reply->host.id
                );
            }
            goto done;
        }
    }

    if (config.workload_sim && g_request->have_other_results_list) {
        init_ip_results(
            g_request->global_prefs.work_buf_min(),
            g_wreq->effective_ncpus, g_request->ip_results
        );
    }

    // send non-CPU-intensive jobs if needed
    //
    if (ssp->have_nci_app) {
        send_nci();
    }

    if (!work_needed(false)) {
        goto done;
    }

    if (config.locality_scheduler_fraction > 0) {
        if (drand() < config.locality_scheduler_fraction) {
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[mixed] sending locality work first\n"
                );
            }
            send_work_locality();

            // save 'insufficient' flags from the first scheduler
            bool disk_insufficient  = g_wreq->disk.insufficient;
            bool speed_insufficient = g_wreq->speed.insufficient;
            bool mem_insufficient   = g_wreq->mem.insufficient;
            bool no_allowed_apps_available = g_wreq->no_allowed_apps_available;

            // reset 'insufficient' flags for the second scheduler
            g_wreq->disk.insufficient = false;
            g_wreq->speed.insufficient = false;
            g_wreq->mem.insufficient = false;
            g_wreq->no_allowed_apps_available = false;

            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[mixed] sending non-locality work second\n"
                );
            }
            send_work_old();

            // recombine the 'insufficient' flags from the two schedulers
            g_wreq->disk.insufficient  = g_wreq->disk.insufficient && disk_insufficient;
            g_wreq->speed.insufficient = g_wreq->speed.insufficient && speed_insufficient;
            g_wreq->mem.insufficient   = g_wreq->mem.insufficient && mem_insufficient;
            g_wreq->no_allowed_apps_available = g_wreq->no_allowed_apps_available && no_allowed_apps_available;

        } else {
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[mixed] sending non-locality work first\n"
                );
            }

            // save 'insufficient' flags from the first scheduler
            bool disk_insufficient  = g_wreq->disk.insufficient;
            bool speed_insufficient = g_wreq->speed.insufficient;
            bool mem_insufficient   = g_wreq->mem.insufficient;
            bool no_allowed_apps_available = g_wreq->no_allowed_apps_available;

            // reset 'insufficient' flags for the second scheduler
            g_wreq->disk.insufficient = false;
            g_wreq->speed.insufficient = false;
            g_wreq->mem.insufficient = false;
            g_wreq->no_allowed_apps_available = false;

            send_work_old();
            if (config.debug_locality) {
                log_messages.printf(MSG_NORMAL,
                    "[mixed] sending locality work second\n"
                );
            }
            send_work_locality();

            // recombine the 'insufficient' flags from the two schedulers
            g_wreq->disk.insufficient  = g_wreq->disk.insufficient && disk_insufficient;
            g_wreq->speed.insufficient = g_wreq->speed.insufficient && speed_insufficient;
            g_wreq->mem.insufficient   = g_wreq->mem.insufficient && mem_insufficient;
            g_wreq->no_allowed_apps_available = g_wreq->no_allowed_apps_available && no_allowed_apps_available;

        }
    } else if (config.locality_scheduling) {
        send_work_locality();
    } else if (config.sched_old) {
        send_work_old();
    } else {
        send_work_score();
    }

done:
    retval = update_host_app_versions(g_reply->results, g_reply->host.id);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "update_host_app_versions() failed: %s\n", boincerror(retval)
        );
    }
    send_user_messages();
}
