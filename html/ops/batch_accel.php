#!/usr/bin/env php
<?php
// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
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

// identify batches that are almost complete, and accelerate their completion
// We first run 'batch_stats.php', which computes needed data:
// LTT hosts, accelerable batches, median TT of batches

require_once('../inc/util.inc');
require_once('../inc/boinc_db.inc');
require_once('../inc/submit_db.inc');
require_once('../inc/submit_util.inc');

ini_set('display_errors', true);

// the following can be set in project.inc
//
// accelerate a batch if at least this frac of jobs are done (success or fail)
// This value is a guess; it may not be optimal.
// If too low, extra instances are created, reducing throughput.
// If too high, batches take longer to finish.
//
if (!defined('BATCH_ACCEL_MIN_FRAC_DONE')) {
    define('BATCH_ACCEL_MIN_FRAC_DONE', 0.85);
}

// accelerate a batch if the number of 'not done' jobs
// (i.e. unsent or in progress) is less than this.
// This helps small batches get done quicker
//
if (!defined('BATCH_ACCEL_MAX_NOT_DONE')) {
    define('BATCH_ACCEL_MAX_NOT_DONE', 20);
}

$apps = [];

// batch is in-progress and qualifies for acceleration (see above);
// accelerate its remaining jobs by marking them as high-priority
// and possibly creating a new instance
//
function do_batch($batch, $wus) {
    global $apps;
    $now = time();
    $app = $apps[$batch->app_id];
    if ($app->n_size_classes) {
        // app is accelerable; mark in-progress WUs as high priority
        //
        foreach ($wus as $wu) {
            if ($wu->canonical_resultid) {
                continue;
            }
            if ($wu->error_mask) {
                continue;
            }
            echo "accelerating WU $wu->id\n";
            $results = BoincResult::enum_fields(
                'id, server_state, sent_time, priority',
                "workunitid=$wu->id"
            );

            // create a new (high priority) result if
            // - we're not at the max # of result
            // - there are no unsent or recently sent results
            //
            $make_another_result = false;
            if (count($results) < $wu->max_total_results) {
                $make_another_result = true;
                foreach ($results as $r) {
                    switch ($r->server_state) {
                    case RESULT_SERVER_STATE_UNSENT:
                        echo "   have unsent result $r->id\n";
                        $make_another_result = false;
                        if ($r->priority == 0) {
                            echo "   boosting its priority\n";
                            $r->update('priority=1');
                        } else {
                            echo "   already high priority\n";
                        }
                        break;
                    case RESULT_SERVER_STATE_IN_PROGRESS:
                        $age = $now - $r->sent_time;
                        if ($age < $batch->expire_time) {
                            echo "   have recent in-progress result $r->id\n";
                            $make_another_result = false;
                        }
                        break;
                    }
                }
            }
            $query = [];
            if ($make_another_result) {
                echo "   creating another instance\n";
                $query[] = sprintf(
                    'target_nresults=%d', $wu->target_nresults+1
                );
                $query[] = sprintf(
                    'max_total_results=%d', $wu->max_total_results+1
                );
                $query[] = sprintf(
                    'transition_time=%f', $now
                );
            }
            if ($wu->priority == 0) {
                echo "   setting WU to high prio\n";
                $query[] = 'priority=1';
            }
            if ($query) {
                $query = implode(',', $query);
                BoincWorkunit::update_aux($query, "id=$wu->id");
            }
        }
    } else {
        // not accelerable; reset job priorities
        echo "batch is not accelerable; resetting job priorities\n";
        BoincWorkunit::update_aux(
            'priority=0',
            sprintf('batch=%d', $batch->id)
        );
        BoincResult::update_aux(
            'priority=0',
            sprintf('batch=%d', $batch->id)
        );
    }
}

function main() {
    global $apps;

    echo sprintf("starting batch_accel: %s\n", time_str(time()));

    $as = BoincApp::enum('');
    foreach ($as as $a) {
        $apps[$a->id] = $a;
    }

    $batches = BoincBatch::enum(
        sprintf('state=%d', BATCH_STATE_IN_PROGRESS)
    );
    foreach ($batches as $batch) {
        $wus = BoincWorkunit::enum_fields(
            'id, name, rsc_fpops_est, canonical_credit, canonical_resultid, error_mask, max_total_results, target_nresults, priority',
            "batch = $batch->id"
        );
        $batch = get_batch_params($batch, $wus);
        if ($batch->state != BATCH_STATE_IN_PROGRESS) {
            echo "batch $batch->id not in progress\n";
            continue;
        }
        $n_done = $batch->njobs_success + $batch->nerror_jobs;
        $n_not_done = count($wus) - $n_done;
        if ($batch->fraction_done > BATCH_ACCEL_MIN_FRAC_DONE
            || $n_not_done < BATCH_ACCEL_MAX_NOT_DONE
        ) {
            echo "doing batch $batch->id\n";
            do_batch($batch, $wus);
        } else {
            echo "not doing batch $batch->id: $batch->fraction_done done\n";
        }
    }
    echo sprintf("finished batch_accel: %s\n", time_str(time()));
}

system("./batch_stats.php");
main();

?>
