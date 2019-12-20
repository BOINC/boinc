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

// structure for tracking the recent approximate median
// of a distribution that may change over time
//
#ifndef _MEDIAN_H
#define _MEDIAN_H

#include <cmath>
#include <algorithm>
#include "average.h"

#ifdef __GNUC__
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

//  warning, this method should not be used for a value that can change sign.
struct MEDIAN : AVERAGE {
    // return true if sample exceeded limit and was truncated
    //
    inline bool update(
        double sample,
        UNUSED double n_threshold,
        // unused.  Here for compatibility with AVERAGE
        double sample_weight,
        // sample weight in the median
        UNUSED double sample_limit
        // unused.  Here for compatibility with AVERAGE
    ) {
        if (n==0) {
            avg=sample;
            n++;
            return false;
        }
        double delta=avg*std::max(sample_weight,1.0/(n+1));
        n++;
        if (sample>avg) {
            if (sample<(avg+delta)) {
                avg=sample;
            } else {
                avg=avg+delta;
            }
        } else {
            if (sample>(avg-delta)) {
                avg=sample;
            } else {
                avg=avg-delta;
            }
        }
        return false;
    }

    inline double get_med() {
        return avg;
    }

    MEDIAN() {} ;
    MEDIAN(const MEDIAN &m) : AVERAGE(m) {} ;

    MEDIAN &operator =(const MEDIAN &m) {
        if (this != &m) {
            n=m.n;
            avg=m.avg;
        }
        return *this;
    }
};

// same, but variance also
//
struct MEDIAN_VAR : MEDIAN {
    double var;
    double q;

    inline bool update_var(
        double sample,
        double n_threshold,
        // unused.  Here for compatibility with AVERAGE
        double sample_weight,
        // sample weight in the median
        double sample_limit
        // unused.  Here for compatibility with AVERAGE
    ) {
        double delta=sample-avg;
        MEDIAN::update(sample,n_threshold,sample_weight,sample_limit);

        double weight=avg*std::max(sample_weight,1.0/n);
        var=var*(1-weight)+delta*delta*weight;
        return false;
    }

    inline double get_var() {
        return var;
    }

    inline void clear() {
        MEDIAN::clear();
        var = 0;
        q = 0;
    }

    MEDIAN_VAR() : MEDIAN() {}
};
#endif
