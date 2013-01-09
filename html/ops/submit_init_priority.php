#! /usr/bin/env php
<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

// initialize
//   user_submit.logical start_time
//   batch.logical_start_time
//   batch.logical_end_time
//   result.priority
// based on existing batches

require_once("../inc/boinc_db.inc");
require_once("../inc/submit_db.inc");

function process_batch($b) {
    $app = BoincApp::lookup_id($b->app_id);
    if (!$app) {
        echo "no app for batch $b->id\n";
        return;
    }
    if ($b->fraction_done>0 && $b->credit_canonical>0) {
        $credit_total = $b->credit_canonical/$b->fraction_done;
        $fpops_total = $credit_total*(86400e9/200);
    } else {
        $db = BoincDb::get();
        $fpops_total = $db->sum(
            "workunit", "rsc_fpops_est*target_nresults", "where batch=$b->id"
        );
    }
    echo "batch $b->id fpops_total $fpops_total\n";
    if ($fpops_total == 0) {
        return;
    }

    // adjust the user's logical start time
    //
    $user = BoincUser::lookup_id($b->user_id);
    if (!$user) die("no user $b->user_id\n");
    $us = BoincUserSubmit::lookup_userid("$user->id");
    if (!$us) die("no user submit record\n");
    $lst = $us->logical_start_time;
    $cmd = "cd ../../bin; ./adjust_user_priority --user $user->id --flops $fpops_total --app $app->name";
    system($cmd);
    $us = BoincUserSubmit::lookup_userid("$user->id");
    $let = $us->logical_start_time;
    $let = (int)$let;

    // set the priority of workunits and results in this batch
    // to the user's new logical start time
    //
    $clause = "priority=$let where batch=$b->id";
    BoincResult::update_aux($clause);
    BoincWorkunit::update_aux($clause);
}

function scan_batches() {
    $batches = BoincBatch::enum("", "order by id");
    foreach ($batches as $b) {
        process_batch($b);
    }
}

function reset_all() {
    BoincUserSubmit::update_aux("logical_start_time=0");
    BoincBatch::update_aux("logical_start_time=0, logical_end_time=0");
    BoincWorkunit::update_aux("priority=0");
    BoincResult::update_aux("priority=0");
}

scan_batches();
//reset_all();

?>
