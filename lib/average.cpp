// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2010 University of California
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

#include "average.h"

bool AVERAGE::update(double sample) {
    double delta, limit;
    bool truncated = false;
    if (sample < 0) return true;
    if (n) {
        if (sample > avg*sample_limit) {
            sample = avg*sample_limit;
            truncated = true;
        }
    }
    n++;
    if (n < n_threshold) {
        avg += (sample-avg)/n;
    } else {
        delta = sample - avg;
        avg += sample_weight*delta;
    }
    return truncated;
}

bool AVERAGE_VAR::update_var(double sample) {
    double delta, limit;
    bool truncated = false;
    if (sample < 0) return true;
    if (n) {
        if (sample > avg*sample_limit) {
            sample = avg*sample_limit;
            truncated = true;
        }
    }
    n++;
    if (n < n_threshold) {
        q += (sample - avg ) * (n - 1) / n;
        avg += (sample-avg)/n;
        var = q/n;
    } else {
        delta = sample - avg;
        avg += sample_weight*delta;
        double vdelta = (delta*delta - var);
        var += sample_weight*vdelta;
    }
    return truncated;
}

#if 0
#include <stdio.h>
#include "util.h"
int main() {
    AVERAGE_VAR avg;
    avg.init(100, .01, 10);

    for (int i=0; i<1000; i++) {
        avg.update_var(drand());
        printf("%d %f %f\n", i, avg.get_avg(), sqrt(avg.get_var()));
    }
}
#endif
