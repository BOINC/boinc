// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/ 
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

using namespace std;
#include <vector>

#include "db.h"
#include "config.h"
#include "parse.h"

extern CONFIG config;

// get the name of a result's (first) output file
//
void get_output_file_path(RESULT& result, char* path) {
    char buf[256];
    int retval;

    strcpy(path, "");
    retval = parse_str(result.xml_doc_in, "<name>", buf, sizeof(buf));
    if (retval) return;
    char* upload_dir = config.upload_dir;
    sprintf(path, "%s/%s", upload_dir, buf);
}

// crude example of a validation function.
// See if there's a strict majority under equality.
//
int check_set(vector<RESULT> results, int& canonical, double& credit) {
    int i, j, n, neq, retval, ilow, ihigh;
    char* files[100];
    char path[256];
    bool found;
    double c, low, high;

    canonical = 0;
    n = results.size();

    // read the result files into malloc'd memory buffers
    //
    for (i=0; i<n; i++) {
        get_output_file_path(results[i], path);
        retval = read_file_malloc(path, files[i]);
        if (retval) {
            fprintf(stderr, "read_file_malloc %s %d\n", path, retval);
            return retval;
        }
    }

    // go through the files, compare with the others.
    // If equal to over half, make it the canonical result
    //
    for (i=0; i<n; i++) {
        neq = 0;
        for (j=0; j<n; j++) {
            if (!strcmp(files[i], files[j])) neq++;
        }
        if (neq > n/2) {
            found = true;
            canonical = results[i].id;
            break;
        }
    }

    // if we have a canonical result, flag all matching results as valid
    // Also find the low and high claimed credits
    //
    ilow = ihigh = -1;
    for (i=0; i<n; i++) {
        if (!strcmp(files[i], files[canonical])) {
            results[i].validate_state = VALIDATE_STATE_VALID;
            c = results[i].claimed_credit;
            if (ilow < 0) {
                ilow = ihigh = i;
                low = high = c;
            } else {
                if (c < low) {
                    low = c;
                    ilow = i;
                }
                if (c > high) {
                    high = c;
                    ihigh = i;
                }
            }
        } else {
            results[i].validate_state = VALIDATE_STATE_INVALID;
        }
    }

    // if we have a canonical result, compute a canonical credit as follows:
    // - if N==1, give that credit
    // - if N==2, give min credit
    // - if N>2, toss out min and max, give average of rest
    //
    if (neq == 1) {
        credit = low;
    } else if (neq == 2) {
        credit = low;
    } else {
        double sum = 0;
        for (i=0; i<n; i++) {
            if (i == ilow) continue;
            if (i == ihigh) continue;
            if (results[i].validate_state == VALIDATE_STATE_VALID) {
                sum += results[i].claimed_credit;
            }
        }
        credit = sum/(neq-2);
    }

    // free malloced files
    //
    for (i=0; i<n; i++) {
        free(files[i]);
    }
    return 0;
}

int check_pair(RESULT& r1, RESULT& r2, bool& match) {
    char path[256];
    char* p1, *p2;
    int retval;

    get_output_file_path(r1, path);
    retval = read_file_malloc(path, p1);
    if (retval) {
        fprintf(stderr, "read_file_malloc %s %d\n", path, retval);
        return retval;
    }
    get_output_file_path(r2, path);
    retval = read_file_malloc(path, p2);
    if (retval) {
        fprintf(stderr, "read_file_malloc %s %d\n", path, retval);
        return retval;
    }
    match = !strcmp(p1, p2);
    free(p1);
    free(p2);
    return 0;
}
