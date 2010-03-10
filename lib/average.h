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

#include <math.h>

// structure for tracking the recent average
// of a distribution that may change over time
//
struct AVERAGE {
    double n;       // double to avoid integer overflow
    double sum;
    double avg;

    double n_threshold;
        // after this many samples, use exponential average
    double sample_weight;
        // new samples get this weight in exp avg
    double sample_limit;
        // truncate samples at avg*limit

    // return true if sample exceeded limit and was truncated
    //
    bool update(double);

    inline double get_avg() {
        return avg;
    }

    inline void init(double n_thresh, double sample_w, double sample_lim) {
        n = 0;
        sum = 0;
        avg = 0;
        n_threshold = n_thresh;
        sample_weight = sample_w;
        sample_limit = sample_lim;
    }
};

// same, but variance also
//
struct AVERAGE_VAR : AVERAGE {
    double var;
    double sum_sq;

    bool update(double);
    inline double get_var() {
        return var;
    }

    inline void init(double n, double w, double l) {
        AVERAGE::init(n,w,l);
        var = 0;
        sum_sq = 0;
    }
};
