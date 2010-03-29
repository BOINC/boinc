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
    double avg;

    // save original values here so you can update DB incrementally
    double n_orig;
    double avg_orig;

    inline void clear() {
        n = avg = 0;
    }

    inline void save_orig() {
        n_orig = n;
        avg_orig = avg;
    }

    inline bool changed() {
        return (n_orig != n);
    }

    // return true if sample exceeded limit and was truncated
    //
    inline bool update(
        double sample,
        double n_threshold,
            // after this many samples, use exponential average
        double sample_weight,
            // new samples get this weight in exp avg
        double sample_limit
            // truncate samples at avg*limit
    ) {
        double delta, limit;
        bool truncated = false;
        if (sample < 0) return true;
        if (n && avg) {
            if (sample > avg*sample_limit) {
                sample = avg*sample_limit;
                truncated = true;
            }
        }
        n++;
        delta = sample - avg;
        if (n < n_threshold) {
            avg += delta/n;
        } else {
            avg += sample_weight*delta;
        }
        return truncated;
    }

    inline double get_avg() {
        return avg;
    }
};

// same, but variance also
//
struct AVERAGE_VAR : AVERAGE {
    double var;
    double q;

    double q_orig;
    double var_orig;

    inline void save_orig_var() {
        n_orig = n;
        avg_orig = avg;
        q_orig = q;
        var_orig = var;
    }

    inline bool update_var(
        double sample,
        double n_threshold,
            // after this many samples, use exponential average
        double sample_weight,
            // new samples get this weight in exp avg
        double sample_limit
            // truncate samples at avg*limit
    ) {
        double delta, limit;
        bool truncated = false;
        if (sample < 0) return true;
        if (n && avg) {
            if (sample > avg*sample_limit) {
                sample = avg*sample_limit;
                truncated = true;
            }
        }
        n++;
        delta = sample - avg;
        if (n < n_threshold) {
            avg += delta/n;
            q += delta*(sample - avg);
            var = q/n;
        } else {
            avg += sample_weight*delta;
            double vdelta = (delta*delta - var);
            var += sample_weight*vdelta;
        }
        return truncated;
    }

    inline double get_var() {
        return var;
    }

    inline void clear() {
        AVERAGE::clear();
        var = 0;
        q = 0;
    }
};
