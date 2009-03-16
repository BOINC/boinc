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

// Code to facilitate writing validators.
// Can be used as the basis for a validator that accepts everything
// (see sample_trivial_validator.C),
// or that requires strict equality (see sample_bitwise_validator.C)
// or that uses fuzzy comparison.

#include <cstring>
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

int FILE_INFO::parse(XML_PARSER& xp) {
    char tag[256];
    bool is_tag, found=false;
    optional = false;
    no_validate = false;
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "/file_ref")) {
            return found?0:ERR_XML_PARSE;
        }
        if (xp.parse_string(tag, "file_name", name)) {
            found = true;
            continue;
        }
        if (xp.parse_bool(tag, "optional", optional)) continue;
        if (xp.parse_bool(tag, "no_validate", no_validate)) continue;
    }
    return ERR_XML_PARSE;
}

int get_output_file_info(RESULT& result, FILE_INFO& fi) {
    char tag[256], path[1024];
    bool is_tag;
    string name;
    MIOFILE mf;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "file_ref")) {
            int retval = fi.parse(xp);
            if (retval) return retval;
            dir_hier_path(
                fi.name.c_str(), config.upload_dir, config.uldl_dir_fanout, path
            );
            fi.path = path;
            return 0;
        }
    }
    return ERR_XML_PARSE;
}

int get_output_file_infos(RESULT& result, vector<FILE_INFO>& fis) {
    char tag[256], path[1024];
    bool is_tag;
    MIOFILE mf;
    string name;
    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);
    fis.clear();
    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "file_ref")) {
            FILE_INFO fi;
            int retval =  fi.parse(xp);
            if (retval) return retval;
            dir_hier_path(
                fi.name.c_str(), config.upload_dir, config.uldl_dir_fanout, path
            );
            fi.path = path;
            fis.push_back(fi);
        }
    }
    return 0;
}

int get_output_file_path(RESULT& result, string& path) {
    FILE_INFO fi;
    int retval = get_output_file_info(result, fi);
    if (retval) return retval;
    path = fi.path;
    return 0;
}

int get_output_file_paths(RESULT& result, vector<string>& paths) {
    vector<FILE_INFO> fis;
    int retval = get_output_file_infos(result, fis);
    if (retval) return retval;
    paths.clear();
    for (unsigned int i=0; i<fis.size(); i++) {
        paths.push_back(fis[i].path);
    }
    return 0;
}

struct FILE_REF {
    char file_name[256];
    char open_name[256];
    int parse(XML_PARSER& xp) {
        char tag[256];
        bool is_tag;

        strcpy(file_name, "");
        strcpy(open_name, "");
        while (!xp.get(tag, sizeof(tag), is_tag)) {
            if (!is_tag) continue;
            if (!strcmp(tag, "/file_ref")) {
                return 0;
            }
            if (xp.parse_str(tag, "file_name", file_name, sizeof(file_name))) continue;
            if (xp.parse_str(tag, "open_name", open_name, sizeof(open_name))) continue;
        }
        return ERR_XML_PARSE;
    }
};

// given a path returned by the above, get the corresponding logical name
//
int get_logical_name(RESULT& result, string& path, string& name) {
    char phys_name[1024];
    char tag[256];
    bool is_tag;
    MIOFILE mf;
    int retval;

    mf.init_buf_read(result.xml_doc_in);
    XML_PARSER xp(&mf);

    strcpy(phys_name, path.c_str());
    char* p = strrchr(phys_name, '/');
    if (!p) return ERR_NOT_FOUND;
    strcpy(phys_name, p+1);

    while (!xp.get(tag, sizeof(tag), is_tag)) {
        if (!is_tag) continue;
        if (!strcmp(tag, "result")) continue;
        if (!strcmp(tag, "file_ref")) {
            FILE_REF fr;
            retval = fr.parse(xp);
            if (retval) continue;
            if (!strcmp(phys_name, fr.file_name)) {
                name = fr.open_name;
                return 0;
            }
            continue;
        }
        xp.skip_unexpected(tag, false, 0);
    }
    return ERR_XML_PARSE;
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

int get_credit_from_wu(WORKUNIT& wu, vector<RESULT>&, double& credit) {
    double x;
    int retval;
    DB_WORKUNIT dbwu;

    dbwu.id = wu.id;
    retval = dbwu.get_field_str("xml_doc", dbwu.xml_doc, sizeof(dbwu.xml_doc));
    if (!retval) {
        if (parse_double(dbwu.xml_doc, "<credit>", x)) {
            credit = x;
            return 0;
        }
    }
    return ERR_XML_PARSE;
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

double stddev_credit(WORKUNIT& wu, std::vector<RESULT>& results) {
    double credit_low_bound = 0, credit_high_bound = 0;
    double penalize_credit_high_bound = 0;
    double credit_avg = 0;
    double credit = 0;
    double old = 0;
    double std_dev = 0;
    int nvalid = 0;
    unsigned int i;

    //calculate average
    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        credit = credit + result.claimed_credit;
        nvalid++;
    }

    if (nvalid == 0) {
        return CREDIT_EPSILON;
    }

    credit_avg = credit/nvalid;

    nvalid = 0;
    //calculate stddev difference
    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        std_dev = pow(credit_avg - result.claimed_credit,2) + std_dev;
        nvalid++;
    }

    std_dev = std_dev/ (double) nvalid;
    std_dev = sqrt(std_dev);

    credit_low_bound = credit_avg-std_dev;
    if (credit_low_bound > credit_avg*.85) {
        credit_low_bound = credit_avg*.85;
    }
    credit_low_bound = credit_low_bound - 2.5;
    if (credit_low_bound < 1) credit_low_bound = 1;

    credit_high_bound = credit_avg+std_dev;
    if (credit_high_bound < credit_avg*1.15) {
        credit_high_bound = credit_avg*1.15;
    }
    credit_high_bound = credit_high_bound + 5;


    nvalid=0;
    credit = 0;
    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        if (result.claimed_credit < credit_high_bound && result.claimed_credit > credit_low_bound) {
            credit = credit + result.claimed_credit;
            nvalid++;
        } else {
            log_messages.printf(MSG_NORMAL,
                "[RESULT#%d %s] CREDIT_CALC_SD Discarding invalid credit %.1lf, avg %.1lf, low %.1lf, high %.1lf \n",
                result.id, result.name, result.claimed_credit,
                credit_avg, credit_low_bound, credit_high_bound
            );
        }
    }

    double grant_credit;
    switch(nvalid) {
    case 0:
        grant_credit = median_mean_credit(wu, results);
        old = grant_credit;
        break;
    default:
        grant_credit = credit/nvalid;
        old = median_mean_credit(wu, results);
    }

    // Log what happened
    if (old > grant_credit) {
        log_messages.printf(MSG_DEBUG,
            "CREDIT_CALC_VAL New Method grant: %.1lf  Old Method grant: %.1lf  Less awarded\n",
            grant_credit, old
        );
    } else if (old == grant_credit) {
        log_messages.printf(MSG_DEBUG,
            "CREDIT_CALC_VAL New Method grant: %.1lf  Old Method grant: %.1lf  Same awarded\n",
            grant_credit, old
        );
    } else {
        log_messages.printf(MSG_DEBUG,
            "CREDIT_CALC_VAL New Method grant: %.1lf  Old Method grant: %.1lf  More awarded\n",
            grant_credit, old
        );
    }

    // penalize hosts that are claiming too much
    penalize_credit_high_bound = grant_credit+1.5*std_dev;
    if (penalize_credit_high_bound < grant_credit*1.65) {
        penalize_credit_high_bound = grant_credit*1.65;
    }
    penalize_credit_high_bound = penalize_credit_high_bound + 20;

    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        if (result.claimed_credit > penalize_credit_high_bound) {
            result.granted_credit = grant_credit * 0.5;
            log_messages.printf(MSG_NORMAL,
                "[RESULT#%d %s] CREDIT_CALC_PENALTY Penalizing host for too high credit %.1lf, grant %.1lf, penalize %.1lf, stddev %.1lf, avg %.1lf, low %.1lf, high %.1lf \n",
                result.id, result.name, result.claimed_credit, grant_credit,
                penalize_credit_high_bound, std_dev, credit_avg,
                credit_low_bound, credit_high_bound
            );
        }
    }

    return grant_credit;
}

double two_credit(WORKUNIT& wu, std::vector<RESULT>& results) {
    int i;
    double credit = 0;
    double credit_avg = 0;
    double last_credit = 0;
    int nvalid = 0;

    // calculate average
    //
    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        credit = credit + result.claimed_credit;
        last_credit = result.claimed_credit;
        nvalid++;
    }

    // If more then 2 valid results, compute via stddev method
    //
    if ( nvalid > 2 ) return stddev_credit(wu, results);

    // This case should never occur
    //
    if (nvalid == 0 ) {
        log_messages.printf(MSG_CRITICAL,
            "[WORKUNIT#%d %s] No valid results\n", wu.id, wu.name
        );
        exit(-1);
    }
   
    credit_avg = credit/nvalid;
   
    // Next check to see if there is reasonably close agreement between the
    // two results.  A study performed at World Community Grid found that in
    // 85% of cases the credit claimed were within 15% of the average claimed
    // credit for the workunit.  Return the average of the claimed credit
    // in these cases.
    //
    if ( fabs(last_credit - credit_avg) < 0.15*credit_avg ) return credit_avg;

    // If we get here, then there was not agreement between the claimed credits
    // So attempt to use the average of the historical granted credit instead
    //
    DB_HOST host;
    double credit_hist_avg=0;
    double credit_min_dev=credit_avg;
        // default award in case nobody matches the cases
    nvalid=0;
    double deviation = -1;
    for (i=0; i<results.size(); i++) {
        RESULT& result = results[i];
        if (result.validate_state != VALIDATE_STATE_VALID) continue;
        host.lookup_id(result.hostid);
        // skip if host is new or the cpu time is very low
        if ( host.total_credit < config.granted_credit_ramp_up
            || result.cpu_time < 30 ) continue;

        // This is for computing the average based on the computers history
        credit_hist_avg = credit_hist_avg + result.cpu_time*host.credit_per_cpu_sec;
        nvalid++;
        last_credit = result.cpu_time*host.credit_per_cpu_sec;

        // This if is for finding the result whose claimed credit is the least
        // different from the computers historical average
        //
        if ( (deviation < 0 || deviation > fabs(result.claimed_credit - result.cpu_time*host.credit_per_cpu_sec))
        ) {
                deviation = fabs(result.claimed_credit - result.cpu_time*host.credit_per_cpu_sec);
                credit_min_dev = result.claimed_credit;
        }
    }
   
    // If this case occurs, then this is becuase neither host has
    // been participating long.  As a result, returned the claimed
    // credit average
    if (nvalid == 0 ) {
        log_messages.printf(MSG_DEBUG,"[WORKUNIT#%d %s] No qualifying results",
            wu.id, wu.name);      
        return credit_avg;
    }

    credit_hist_avg = credit_hist_avg/nvalid;

   
    // Check to see if the result.cpu_time*host.credit_per_cpu_sec are close.
    // If so use the average of the historical credit
    //
    if (fabs(last_credit-credit_hist_avg)<0.1*credit_hist_avg) {
        log_messages.printf(MSG_DEBUG,"[WORKUNIT#%d %s] Method1: "
            "Credit Average = %.2lf Actual Credit Granted = %.2lf \n",
            wu.id, wu.name, credit_avg, credit_hist_avg);                
        return credit_hist_avg;
    }

    log_messages.printf(MSG_DEBUG,"[WORKUNIT#%d %s] Method2: "
        "Credit Average = %.2lf Actual Credit Granted = %.2lf \n",
        wu.id, wu.name, credit_avg, credit_min_dev);                

    return credit_min_dev;
}

const char *BOINC_RCSID_07049e8a0e = "$Id$";
