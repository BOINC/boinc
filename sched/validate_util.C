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

// Code to facilitate writing validators.
// Can be used as the basis for a validator that accepts everything
// (see sample_trivial_validator.C),
// or that requires strict equality (see sample_bitwise_validator.C)
// or that uses fuzzy comparison.

#include "config.h"

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "sched_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "validator.h"
#include "validate_util.h"

using std::vector;
using std::string;

static int parse_filename(XML_PARSER& xp, string& name) {
    char tag[256];
    bool is_tag, found=false;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/file_info")) {
            return found?0:ERR_XML_PARSE;
        }
        if (xp.parse_string(tag, "name", name)) {
            found = true;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file_path(RESULT const& result, string& path_str) {
    char tag[256], path[1024];
    bool is_tag;
    string name;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_out);
    XML_PARSER xp(&mf);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "file_info")) {
            int retval = parse_filename(xp, name);
            if (retval) return retval;
            dir_hier_path(name.c_str(), config.upload_dir, config.uldl_dir_fanout, path);
            path_str = path;
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file_paths(RESULT const& result, vector<string>& paths) {
    char tag[256], path[1024];
    bool is_tag;
    MIOFILE mf;
    string name;
    mf.init_buf_read(result.xml_doc_out);
    XML_PARSER xp(&mf);
    paths.clear();
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "file_info")) {
            int retval =  parse_filename(xp, name);
            if (retval) return retval;
            dir_hier_path(name.c_str(), config.upload_dir, config.uldl_dir_fanout, path);
            paths.push_back(path);
        }
    }
    return 0;
}

#define CREDIT_EPSILON .001

// If we have N correct results with nonzero claimed credit,
// compute a canonical credit as follows:
// - if N==0 (all claimed credits are infinitesmal), return CREDIT_EPSILON
// - if N==1, return that credit
// - if N==2, return min
// - if N>2, toss out min and max, return average of rest
//
double median_mean_credit(WORKUNIT& /*wu*/, vector<RESULT>& results) {
    int ilow=-1, ihigh=-1;
    double credit_low = 0, credit_high = 0;
    int nvalid = 0;
    unsigned int i;

    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        if (result.claimed_credit < CREDIT_EPSILON) continue;
        if (ilow < 0) {
            ilow = ihigh = i;
            credit_low = credit_high = result.claimed_credit;
        } else {
            if (result.claimed_credit < credit_low) {
                ilow = i;
                credit_low = result.claimed_credit;
            }
            if (result.claimed_credit > credit_high) {
                ihigh = i;
                credit_high = result.claimed_credit;
            }
        }
        nvalid++;
    }

    switch(nvalid) {
    case 0:
        return CREDIT_EPSILON;
    case 1:
    case 2:
        return credit_low;
    default:
        double sum = 0;
        for (i=0; i<results.size(); i++) {
            if (i == (unsigned int) ilow) continue;
            if (i == (unsigned int) ihigh) continue;
            RESULT& result = results[i];
            if (result.validate_state != VALIDATE_STATE_VALID) continue;

            sum += result.claimed_credit;
        }
        return sum/(nvalid-2);
    }
}

double get_credit_from_wu(WORKUNIT& wu, vector<RESULT>&) {
    double x;
    int retval;
    DB_WORKUNIT dbwu;

    dbwu.id = wu.id;
    retval = dbwu.get_field_str("xml_doc", dbwu.xml_doc, sizeof(dbwu.xml_doc));
    if (!retval) {
        if (parse_double(dbwu.xml_doc, "<credit>", x)) {
            return x;
        }
    }
    fprintf(stderr, "ERROR: <credit> missing from WU XML\n");
    exit(1);
}

// This function should be called from the validator whenever credit
// is granted to a host.  It's purpose is to track the average credit
// per cpu time for that host.
//
// It updates an exponentially-decaying estimate of credit_per_cpu_sec
// Note that this does NOT decay with time, but instead decays with
// total credits earned.  If a host stops earning credits, then this
// quantity stops decaying.  So credit_per_cpu_sec must NOT be
// periodically decayed using the update_stats utility or similar
// methods.
//
// The intended purpose is for cross-project credit comparisons on
// BOINC statistics pages, for hosts attached to multiple machines.
// One day people will write PhD theses on how to normalize credit
// values to equalize them across projects.  I hope this will be done
// according to "Allen's principle": "Credits granted by a project
// should be normalized so that, averaged across all hosts attached to
// multiple projects, projects grant equal credit per cpu second."
// This principle ensures that (on average) participants will choose
// projects based on merit, not based on credits.  It also ensures
// that (on average) host machines migrate to the projects for which
// they are best suited.
//
// For cross-project comparison the value of credit_per_cpu_sec should
// be exported in the statistics file host_id.gz, which is written by
// the code in db_dump.C.
//
// Algorithm: credits_per_cpu_second should be updated each time that
// a host is granted credit, according to:
//
//     CREDIT_AVERAGE_CONST = 500           [see Note 5]
//     MAX_CREDIT_PER_CPU_SEC = 0.1         [see Note 6]
//
//     e = tanh(granted_credit/CREDIT_AVERAGE_CONST)
//     if (e < 0) then e = 0
//     if (e > 1) then e = 1
//     if (credit_per_cpu_sec <= 0) then e = 1
//     if (cpu_time <= 0) then e = 0        [see Note 4]
//     if (granted_credit <= 0) then e = 0  [see Note 3]
//
//     rate = granted_credit/cpu_time
//     if (rate < 0) rate = 0
//     if (rate > MAX_CREDIT_PER_CPU_SEC) rate = MAX_CREDIT_PER_CPU_SEC
//  
//     credit_per_cpu_sec = e * rate + (1 - e) * credit_per_cpu_sec

// Note 0: all quantities above should be treated as real numbers
// Note 1: cpu_time is measured in seconds
// Note 2: When a host is created, the initial value of
//         credit_per_cpu_sec, should be zero.
// Note 3: If a host has done invalid work (granted_credit==0) we have
//         chosen not to include it.  One might argue that the
//         boundary case granted_credit==0 should be treated the same
//         as granted_credit>0.  However the goal here is not to
//         identify cpus whose host machines sometimes produce
//         rubbish.  It is to get a measure of how effectively the cpu
//         runs the application code.
// Note 4: e==0 means 'DO NOT include the first term on the rhs of the
//         equation defining credit_per_cpu_sec' which is equivalent
//         to 'DO NOT update credit_per_cpu_sec'.
// Note 5: CREDIT_AVERAGE_CONST determines the exponential decay
//         credit used in averaging credit_per_cpu_sec.  It may be
//         changed at any time, even if the project database has
//         already been populated with non-zero values of
//         credit_per_cpu_sec.
// Note 6: Typical VERY FAST cpus have credit_per_cpu_sec of around
//         0.02.  This is a safety mechanism designed to prevent
//         trouble if a client or host has reported absurd values (due
//         to a bug in client or server software or by cheating).  In
//         five years when cpus are five time faster, please increase
//         the value of R.  You may also want to increase the value of
//         CREDIT_AVERAGE_CONST.
//
//         Nonzero return value: host exceeded the max allowed
//         credit/cpu_sec.
//
int update_credit_per_cpu_sec(
    double  granted_credit,     // credit granted for this work
    double  cpu_time,           // cpu time (seconds) used for this work
    double& credit_per_cpu_sec  // (average) credit per cpu second
) {
    int retval = 0;

    // Either of these values may be freely changed in the future.
    // When CPUs get much faster one must increase the 'sanity-check'
    // value of max_credit_per_cpu_sec.  At that time it would also
    // make sense to proportionally increase the credit_average_const.
    //
    const double credit_average_const = 500;
    const double max_credit_per_cpu_sec = 0.07;
    
    double e = tanh(granted_credit/credit_average_const);
    if (e <= 0.0 || cpu_time == 0.0 || granted_credit == 0.0) return retval;
    if (e > 1.0 || credit_per_cpu_sec == 0.0) e = 1.0;
    
    double rate =  granted_credit/cpu_time;
    if (rate < 0.0) rate = 0.0;
    if (rate > max_credit_per_cpu_sec) {
        rate = max_credit_per_cpu_sec;
        retval = 1;
    }

    credit_per_cpu_sec = e * rate + (1.0 - e) * credit_per_cpu_sec;

    return retval;
}

const char *BOINC_RCSID_07049e8a0e = "$Id$";
