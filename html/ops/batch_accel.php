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

require_once('../inc/boinc_db.inc');
require_once('../inc/submit_db.inc');
require_once('../inc/submit_util.inc');

ini_set('display_errors', true);

// accelerate batches if at least this frac done
//
define('MIN_FRAC_DONE', .9);

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
            echo "looking at WU $wu->id\n";
            $results = BoincResult::enum_fields(
                'id, server_state, sent_time',
                "workunitid=$wu->id"
            );
            $make_another_result = false;
            if (count($results) < $wu->max_total_results) {
                $make_another_result = true;
                foreach ($results as $r) {
                    if ($r->server_state == RESULT_SERVER_STATE_UNSENT) {
                        echo "unsent result $r->id\n";
                        $make_another_result = false;
                        break;
                    }
                    $age = $now - $r->sent_time;
                    if ($age<$batch->expire_time) {
                        $make_another_result = false;
                        break;
                    }
                }
            }
            $query = [];
            if ($make_another_result) {
                echo "creating another instance for WU $wu->id\n";
                $query[] = sprintf(
                    'target_nresults=%d', $wu->target_nresults+1
                );
                $query[] = sprintf(
                    'transition_time=%f', $now
                );
            }
            if (!$wu->priority) {
                echo "setting WU $wu->id to high prio\n";
                $query[] = 'priority=1';
            }
            if ($query) {
                $query = implode(',', $query);
                BoincWorkunit::update_aux($query, "id=$wu->id");
            }

            echo "boosting results for WU $wu->id\n";
            BoincResult::update_aux(
                'priority=1',
                sprintf('priority=0 and server_state=%d and workunitid=%d',
                    RESULT_SERVER_STATE_UNSENT, $wu->id
                )
            );
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
        if ($batch->fraction_done < MIN_FRAC_DONE) {
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
