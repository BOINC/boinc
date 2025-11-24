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

require_once('../inc/util.inc');
require_once('../inc/boinc_db.inc');
require_once('../inc/submit_db.inc');
require_once('../inc/submit_util.inc');

ini_set('display_errors', true);

// accelerate batches if at least this frac done
// can be set in project.inc
//
if (!defined('BATCH_ACCEL_MIN_FRAC_DONE')){
    define('BATCH_ACCEL_MIN_FRAC_DONE', .85);
}

$apps = [];

// batch is in-progress and at least 90% done; accelerate its remaining jobs
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
                        if ($age<$batch->expire_time) {
                            echo "   have recent in-progress result\n";
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
        if ($batch->fraction_done < BATCH_ACCEL_MIN_FRAC_DONE) {
            echo "batch $batch->id only $batch->fraction_done done\n";
            continue;
        }
        echo "doing batch $batch->id\n";
        do_batch($batch, $wus);
    }
    echo sprintf("finished batch_accel: %s\n", time_str(time()));
}

system("./batch_stats.php");
main();

?>
