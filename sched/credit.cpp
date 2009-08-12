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
// Modify claimed credit based on the historical granted credit if
// the project is configured to do this
//

// functions related to the computation and granting of credit
// Note: this is credit.cpp rather than sched_credit.cpp
// because you might grant credit e.g. from a trickle handler

#include <math.h>

#include "boinc_db.h"

#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "credit.h"

double fpops_to_credit(double fpops, double intops) {
    // TODO: use fp_weight if specified in config file
    double fpc = (fpops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    double intc = (intops/1e9)*COBBLESTONE_FACTOR/SECONDS_PER_DAY;
    return std::max(fpc, intc);
}

double credit_multiplier(int appid, time_t create_time) {
    DB_CREDIT_MULTIPLIER mult;
    mult.get_nearest(appid, create_time);
    return mult.multiplier;
}

static void modify_credit_rating(HOST& host) {
    double new_claimed_credit = 0;
    double percent_difference = 0;
    // The percent difference between claim and history
    double difference_weight = 1;
    // The weight to be applied based on the difference between claim and
    // history
    double credit_weight = 1;
    // The weight to be applied based on how much credit the host has earned
    // (hosts that are new do not have accurate histories so they shouldn't
    // have much weight)
    double combined_weight = 1;

    // Only modify if the credit_per_cpu_sec is established
    // and the option is enabled
    //
    if (host.credit_per_cpu_sec > 0
        && config.granted_credit_weight > 0.0
        && config.granted_credit_weight <= 1.0
    ) {

        // Calculate the difference between claimed credit and the hosts
        // historical granted credit history
        percent_difference=host.claimed_credit_per_cpu_sec-host.credit_per_cpu_sec;
        percent_difference = fabs(percent_difference/host.credit_per_cpu_sec);

        // A study on World Community Grid determined that 50% of hosts
        // claimed within 10% of their historical credit per cpu sec.
        // These hosts should not have their credit modified.
        //
        if (percent_difference < 0.1) {
             log_messages.printf(MSG_DEBUG,
                 "[HOSTID:%d] Claimed credit %.1lf not "
                 "modified.  Percent Difference %.4lf\n", host.id,
                 host.claimed_credit_per_cpu_sec*86400, percent_difference
             );
            return;
        }

        // The study also determined that 95% of hosts claim within
        // 50% of their historical credit per cpu sec.
        // Computers claiming above 10% but below 50% should have their
        // credit adjusted based on their history
        // Computers claiming more than 50% above should use their
        // historical value.
        if (percent_difference < .5) {
            // weight based on variance from historical credit
            difference_weight = 1-(0.5-percent_difference)/0.4;
        } else {
            difference_weight = 1;
        }

        // A weight also needs to be calculated based upon the amount of
        // credit awarded to a host.  This is becuase hosts without much
        // credit awarded do not yet have an accurate history so the weight
        // should be limited for these hosts.
        if (config.granted_credit_ramp_up) {
            credit_weight=config.granted_credit_ramp_up - host.total_credit;
            credit_weight=credit_weight/config.granted_credit_ramp_up;
            if (credit_weight < 0) credit_weight = 0;
            credit_weight = 1 - credit_weight;
        }

        // Compute the combined weight
        combined_weight = credit_weight*difference_weight*config.granted_credit_weight;
        log_messages.printf(MSG_DEBUG, "[HOSTID:%d] Weight details: "
            "diff_weight=%.4lf credit_weight=%.4lf config_weight=%.4lf\n",
            host.id, difference_weight, credit_weight,
            config.granted_credit_weight
        );

        // Compute the new value for claimed credit
        new_claimed_credit=(1-combined_weight)*host.claimed_credit_per_cpu_sec;
        new_claimed_credit=new_claimed_credit+combined_weight*host.credit_per_cpu_sec;

        if (new_claimed_credit < host.claimed_credit_per_cpu_sec) {
            log_messages.printf(MSG_DEBUG,
                "[HOSTID:%d] Modified claimed credit "
                 "(lowered) original: %.1lf new: %.1lf historical: %.1lf "
                 "combined weight: %.4lf\n", host.id,
                 host.claimed_credit_per_cpu_sec*86400,
                 new_claimed_credit*86400, host.credit_per_cpu_sec*86400,
                 combined_weight
            );
        } else {
            log_messages.printf(MSG_DEBUG,
                "[HOSTID:%d] Modified claimed credit "
                "(increased) original: %.1lf new: %.1lf historical: %.1lf "
                "combined weight: %.4lf\n", host.id,
                host.claimed_credit_per_cpu_sec*86400,
                new_claimed_credit*86400, host.credit_per_cpu_sec*86400,
                combined_weight
            );
        }

        host.claimed_credit_per_cpu_sec = new_claimed_credit;
    }
}

// somewhat arbitrary formula for credit as a function of CPU time.
// Could also include terms for RAM size, network speed etc.
//
void compute_credit_rating(HOST& host) {
    double fpw, intw, scale, x;
    if (config.use_benchmark_weights) {

        fpw = config.fp_benchmark_weight;
        intw = 1. - fpw;

        // FP benchmark is 2x int benchmark, on average.
        // Compute a scaling factor the gives the same credit per day
        // no matter how benchmarks are weighted
        //
        scale = 1.5 / (2*intw + fpw);
    } else {
        fpw = .5;
        intw = .5;
        scale = 1;
    }
    x = fpw*fabs(host.p_fpops) + intw*fabs(host.p_iops);
    x /= 1e9;
    x *= COBBLESTONE_FACTOR;
    x /= SECONDS_PER_DAY;
    x *= scale;
    host.claimed_credit_per_cpu_sec  = x;

    if (config.granted_credit_weight) {
        modify_credit_rating(host);
    }
}

