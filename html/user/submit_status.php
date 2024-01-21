<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2011 University of California
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

// web interfaces for viewing and controlling batches
// DEPRECATED: replaced by submit.php

require_once("../inc/util.inc");
require_once("../inc/boinc_db.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_db.inc");

function show_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch || $batch->user_id != $user->id) {
        error_page("no batch");
    }
    page_head("Batch $batch->id");
    $results = BoincResult::enum("batch=$batch->id order by workunitid");
    result_table_start(true, true, null);
    foreach ($results as $result) {
        show_result_row($result, true, true, true);
    }
    end_table();
    page_tail();
}

function show_batches($user) {
    $batches = BoincBatch::enum("user_id=$user->id");
    page_head("Batches");
    start_table();
    table_header("Batch ID", "Submitted", "# jobs");
    foreach ($batches as $batch) {
        echo "<tr>
            <td><a href=submit_status.php?action=show_batch&batch_id=$batch->id>$batch->id</a></td>
            <td>".time_str($batch->create_time)."</td>
            <td>$batch->njobs</td>
            </tr>
        ";
    }
    end_table();
    page_tail();
}

$user = get_logged_in_user();

$action = get_str('action', true);
switch ($action) {
case '': show_batches($user); break;
case 'show_batch': show_batch($user);
}
?>
