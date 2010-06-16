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

#include "boinc_db.h"
#include "sched_types.h"

struct GPU_REQUIREMENTS {
    double min_ram;
    double opt_ram;
    int min_driver_version;
    int opt_driver_version;

    void clear() {
        min_ram = opt_ram = 0;
        min_driver_version = opt_driver_version = 0;
    }
    void update(int version, double ram) {
        if (min_driver_version) {
            if (version < min_driver_version) {
                min_driver_version = version;
            }
        } else {
            min_driver_version = version;
        }
        if (version > opt_driver_version) {
            opt_driver_version = version;
        }
        if (min_ram) {
            if (ram < min_ram) {
                min_ram = ram;
            }
        } else {
            min_ram = ram;
        }
        if (ram > opt_ram) {
            opt_ram = ram;
        }
    }
};

extern GPU_REQUIREMENTS cuda_requirements;
extern GPU_REQUIREMENTS ati_requirements;

extern bool wu_is_infeasible_custom(WORKUNIT&, APP&, BEST_APP_VERSION&);
extern bool app_plan(SCHEDULER_REQUEST&, char* plan_class, HOST_USAGE&);
extern bool app_plan_uses_gpu(const char* plan_class);
