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
    double x = (n > n_threshold) ? avg : sum/n;
    if (sample > x*sample_limit) {
        sample = x*sample_limit;
        truncated = true;
    }
    n++;
    sum += sample;
    if (n < n_threshold) {
        avg = sum/n;
    } else {
        delta = sample - avg;
        avg += sample_weight*delta;
    }
    return truncated;
}

bool AVERAGE_VAR::update(double sample) {
    double delta, limit;
    bool truncated = false;
    if (sample < 0) return true;
    double x = (n > n_threshold) ? avg : sum/n;
    if (sample > x*sample_limit) {
        sample = x*sample_limit;
        truncated = true;
    }
    n++;
    sum += sample;
    sum_sq += sample*sample;
    if (n < n_threshold) {
        avg = sum/n;
        var = sum_sq/n - (avg*avg);
    } else {
        delta = sample - avg;
        avg += sample_weight*delta;
        double vdelta = (delta*delta - var);
        var += sample_weight*vdelta;
    }
    return truncated;
}

#if 0
#include "util.h"
int main() {
    AVERAGE_VAR avg(100, .01, 10);

    for (int i=0; i<1000; i++) {
        avg.update(drand());
        printf("%d %f %f\n", i, avg.get_avg(), sqrt(avg.get_var()));
    }
}
#endif
