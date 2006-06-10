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
// (see validate_trivial.C),
// or that requires strict or fuzzy equality.

#include "config.h"

#include "error_numbers.h"
#include "parse.h"
#include "util.h"
#include "filesys.h"

#include "sched_util.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "validate_util.h"

using std::vector;
using std::string;

extern SCHED_CONFIG config;

// get the name of a result's (first) output file
//
int get_output_file_path(RESULT const& result, string& path_str) {
    char buf[256], path[256];

    if (!parse_str(result.xml_doc_out, "<name>", buf, sizeof(buf))) {
        return ERR_XML_PARSE;
    }
    dir_hier_path(buf, config.upload_dir, config.uldl_dir_fanout, path);
    path_str = path;
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
double median_mean_credit(vector<RESULT>& results) {
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

const char *BOINC_RCSID_07049e8a0e = "$Id$";
