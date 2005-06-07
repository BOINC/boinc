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

#ifndef _TIME_STATS_
#define _TIME_STATS_

#include "miofile.h"

class TIME_STATS {
    double last_update;
    bool first;
public:
// we maintain an exponentially weighted average of these quantities:
    double on_frac;
        // the fraction of total time this host runs the core client
    double connected_frac;
        // of the time this host runs the core client,
        // the fraction it is connected to the Internet,
        // or -1 if not known
    double active_frac;
        // of the time this host runs the core client,
        // the fraction it is enabled to work
        // (as determined by preferences, manual suspend/resume, etc.)

    void update(bool is_active);

    TIME_STATS();
    int write(MIOFILE&, bool to_server);
    int parse(MIOFILE&);
};

#endif
