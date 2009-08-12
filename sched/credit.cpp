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

// This function should be called from the validator whenever credit
// is granted to a host.  It's purpose is to track the average credit
// per cpu time for that host.
//
// It updates an exponentially-decaying estimate of credit_per_cpu_sec
// Note that this does NOT decay with time, but instead decays with
// total credits earned.  If a host stops earning credits, then this
// quantity stops decaying.  So credit_per_cpu_sec must NOT be
// periodically decayed using the update_stats utility or similar
// methods.
//
// The intended purpose is for cross-project credit comparisons on
// BOINC statistics pages, for hosts attached to multiple machines.
// One day people will write PhD theses on how to normalize credit
// values to equalize them across projects.  I hope this will be done
// according to "Allen's principle": "Credits granted by a project
// should be normalized so that, averaged across all hosts attached to
// multiple projects, projects grant equal credit per cpu second."
// This principle ensures that (on average) participants will choose
// projects based on merit, not based on credits.  It also ensures
// that (on average) host machines migrate to the projects for which
// they are best suited.
//
// For cross-project comparison the value of credit_per_cpu_sec should
// be exported in the statistics file host_id.gz, which is written by
// the code in db_dump.C.
//
// Algorithm: credits_per_cpu_second should be updated each time that
// a host is granted credit, according to:
//
//     CREDIT_AVERAGE_CONST = 500           [see Note 5]
//     MAX_CREDIT_PER_CPU_SEC = 0.1         [see Note 6]
//
//     e = tanh(granted_credit/CREDIT_AVERAGE_CONST)
//     if (e < 0) then e = 0
//     if (e > 1) then e = 1
//     if (credit_per_cpu_sec <= 0) then e = 1
//     if (cpu_time <= 0) then e = 0        [see Note 4]
//     if (granted_credit <= 0) then e = 0  [see Note 3]
//
//     rate = granted_credit/cpu_time
//     if (rate < 0) rate = 0
//     if (rate > MAX_CREDIT_PER_CPU_SEC) rate = MAX_CREDIT_PER_CPU_SEC
//
//     credit_per_cpu_sec = e * rate + (1 - e) * credit_per_cpu_sec

// Note 0: all quantities above should be treated as real numbers
// Note 1: cpu_time is measured in seconds
// Note 2: When a host is created, the initial value of
//         credit_per_cpu_sec, should be zero.
// Note 3: If a host has done invalid work (granted_credit==0) we have
//         chosen not to include it.  One might argue that the
//         boundary case granted_credit==0 should be treated the same
//         as granted_credit>0.  However the goal here is not to
//         identify cpus whose host machines sometimes produce
//         rubbish.  It is to get a measure of how effectively the cpu
//         runs the application code.
// Note 4: e==0 means 'DO NOT include the first term on the rhs of the
//         equation defining credit_per_cpu_sec' which is equivalent
//         to 'DO NOT update credit_per_cpu_sec'.
// Note 5: CREDIT_AVERAGE_CONST determines the exponential decay
//         credit used in averaging credit_per_cpu_sec.  It may be
//         changed at any time, even if the project database has
//         already been populated with non-zero values of
//         credit_per_cpu_sec.
// Note 6: Typical VERY FAST cpus have credit_per_cpu_sec of around
//         0.02.  This is a safety mechanism designed to prevent
//         trouble if a client or host has reported absurd values (due
//         to a bug in client or server software or by cheating).  In
//         five years when cpus are five time faster, please increase
//         the value of R.  You may also want to increase the value of
//         CREDIT_AVERAGE_CONST.
//
//         Nonzero return value: host exceeded the max allowed
//         credit/cpu_sec.
//
int update_credit_per_cpu_sec(
    double  granted_credit,     // credit granted for this work
    double  cpu_time,           // cpu time (seconds) used for this work
    double& credit_per_cpu_sec  // (average) credit per cpu second
) {
    int retval = 0;

    // Either of these values may be freely changed in the future.
    // When CPUs get much faster one must increase the 'sanity-check'
    // value of max_credit_per_cpu_sec.  At that time it would also
    // make sense to proportionally increase the credit_average_const.
    //
    const double credit_average_const = 500;
    const double max_credit_per_cpu_sec = 0.07;

    double e = tanh(granted_credit/credit_average_const);
    if (e <= 0.0 || cpu_time == 0.0 || granted_credit == 0.0) return retval;
    if (e > 1.0 || credit_per_cpu_sec == 0.0) e = 1.0;

    double rate =  granted_credit/cpu_time;
    if (rate < 0.0) rate = 0.0;
    if (rate > max_credit_per_cpu_sec) {
        rate = max_credit_per_cpu_sec;
        retval = 1;
    }

    credit_per_cpu_sec = e * rate + (1.0 - e) * credit_per_cpu_sec;

    return retval;
}

// Grant the host (and associated user and team)
// the given amount of credit for a job (or part of a job)
// that started at the given time
//
int grant_credit(
    DB_HOST& host, double start_time, double cpu_time, double credit
) {
    DB_USER user;
    DB_TEAM team;
    int retval;
    char buf[256];

    // first, process the host

    update_average(
        start_time, credit, CREDIT_HALF_LIFE,
        host.expavg_credit, host.expavg_time
    );
    // compute new credit per CPU time
    //
    retval = update_credit_per_cpu_sec(
        credit, cpu_time, host.credit_per_cpu_sec
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "[HOST#%d] claimed too much credit (%f) in too little CPU time (%f)\n",
            host.id, credit, cpu_time
        );
    }
    sprintf(
        buf,
        "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f, credit_per_cpu_sec=%f",
        credit, host.expavg_credit, host.expavg_time,
        host.credit_per_cpu_sec
    );
    retval = host.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "update of host %d failed %d\n",
            host.id, retval
        );
    }

    // then the user

    retval = user.lookup_id(host.userid);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "lookup of user %d failed %d\n",
            host.userid, retval
        );
        return retval;
    }

    update_average(
        start_time, credit, CREDIT_HALF_LIFE,
        user.expavg_credit, user.expavg_time
    );
    sprintf(
        buf, "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f",
        credit,  user.expavg_credit, user.expavg_time
    );
    retval = user.update_field(buf);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "update of user %d failed %d\n",
             host.userid, retval
        );
    }

    // and finally the team

    if (user.teamid) {
        retval = team.lookup_id(user.teamid);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "lookup of team %d failed %d\n",
                user.teamid, retval
            );
            return retval;
        }
        update_average(
            start_time, credit, CREDIT_HALF_LIFE,
            team.expavg_credit, team.expavg_time
        );
        sprintf(buf,
            "total_credit=total_credit+%f, expavg_credit=%f, expavg_time=%f",
            credit,  team.expavg_credit, team.expavg_time
        );
        retval = team.update_field(buf);
        if (retval) {
            log_messages.printf(MSG_CRITICAL,
                "update of team %d failed %d\n",
                team.id, retval
            );
        }
    }
    return 0;
}
