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

#ifndef __RR_SIM__
#define __RR_SIM__

struct RESULT;

struct RR_SIM_PROJECT_STATUS {
        /// jobs currently running (in simulation)
    std::vector<RESULT*>active;
        /// jobs runnable but not running yet
    std::vector<RESULT*>pending;
    int deadlines_missed;
        /// fraction of each CPU this project will get
        /// set in CLIENT_STATE::rr_misses_deadline();
    double proc_rate;
	double active_ncpus;
    double cpu_shortfall;
	bool has_coproc_jobs;
	bool has_cpu_jobs;

    inline void clear() {
        active.clear();
        pending.clear();
        deadlines_missed = 0;
        proc_rate = 0;
        cpu_shortfall = 0;
		active_ncpus = 0;
		has_coproc_jobs = false;
		has_cpu_jobs = false;
    }
    void activate(RESULT* rp);
    inline void add_pending(RESULT* rp) {
        pending.push_back(rp);
    }
    inline bool none_active() {
        return !active.size();
    }
    inline bool none_pending() {
        return !pending.size();
    }
	bool can_run(RESULT* rp, int ncpus);
    void remove_active(RESULT* r);
    inline RESULT* get_pending() {
        if (!pending.size()) return NULL;
        RESULT* rp = pending[0];
        pending.erase(pending.begin());
        return rp;
    }
    inline double cpus_used() {
        return active_ncpus;
    }
};

#endif
