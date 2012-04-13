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

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");
require_once("../project/project.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

// the job submission "home page":
// show the user's in-progress and completed batches,
// and a button for creating a new batch
//
function handle_main($user) {
    page_head("Job submission and control");

    $first = true;
    $batches = BoincBatch::enum("user_id = $user->id order by id desc");

    foreach ($batches as $batch) {
        if ($batch->state < BATCH_STATE_COMPLETE) {
            $wus = BoincWorkunit::enum("batch = $batch->id");
            $batch = get_batch_params($batch, $wus);
        }
        $app = BoincApp::lookup_id($batch->app_id);
        if ($app) {
            $batch->app_name = $app->name;
        } else {
            $batch->app_name = "unknown";
        }
    }

    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_IN_PROGRESS) continue;
        if ($first) {
            $first = false;
            echo "<h2>Batches in progress</h2>\n";
            start_table();
            table_header("name", "ID", "app", "# jobs", "progress", "submitted");
        }
        $pct_done = (int)($batch->fraction_done*100);
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->app_name,
            $batch->njobs,
            "$pct_done%",
            local_time_str($batch->create_time)
        );
    }
    if ($first) {
        echo "<p>You have no in-progress batches.\n";
    } else {
        end_table();
    }

    $first = true;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_COMPLETE) continue;
        if ($first) {
            $first = false;
            echo "<h2>Completed batches</h2>\n";
            start_table();
            table_header("name", "ID", "app", "# jobs", "submitted");
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time)
        );
    }
    if ($first) {
        echo "<p>You have no completed batches.\n";
    } else {
        end_table();
    }

    $first = true;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_ABORTED) continue;
        if ($first) {
            $first = false;
            echo "<h2>Aborted batches</h2>\n";
            start_table();
            table_header("name", "ID", "app", "# jobs", "submitted");
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time)
        );
    }
    if (!$first) {
        end_table();
    }

    echo "<p>
        <a href=sandbox.php><strong> File sandbox </strong></a>
        | <a href=lammps.php><strong> Job submission </strong></a>
        | <a href=submit.php><strong> Job control </strong></a>";
    page_tail();
}

// show the details of an existing batch
//
function handle_query_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    $app = BoincApp::lookup_id($batch->app_id);

    page_head("Batch $batch_id");
    start_table();
    row2("name", $batch->name);
    row2("application", $app->name);
    row2("state", batch_state_string($batch->state));
    row2("# jobs", $batch->njobs);
    row2("# error jobs", $batch->nerror_jobs);
    row2("progress", sprintf("%.0f%%", $batch->fraction_done*100));
    if ($batch->completion_time) {
        row2("completed", local_time_str($batch->completion_time));
    }
    row2("GFLOP/hours, estimated", number_format(credit_to_gflop_hours($batch->credit_estimate), 2));
    row2("GFLOP/hours, actual", number_format(credit_to_gflop_hours($batch->credit_canonical), 2));
    end_table();
    $url = boinc_get_output_files_url($user, $batch_id);
    show_button($url, "Get zipped output files");
    switch ($batch->state) {
    case BATCH_STATE_IN_PROGRESS:
        echo "<br>";
        show_button(
            "submit.php?action=abort_batch_confirm&batch_id=$batch_id",
            "Abort batch"
        );
        break;
    case BATCH_STATE_COMPLETE:
    case BATCH_STATE_ABORTED:
        echo "<br>";
        show_button(
            "submit.php?action=retire_batch_confirm&batch_id=$batch_id",
            "Retire batch"
        );
        break;
    }
    
    echo "<h2>Jobs</h2>\n";
    start_table();
    table_header(
        "Job ID<br><span class=note>click for details or to get output files</span>",
        "status",
        "Canonical instance<br><span class=note>click to see result page on BOINC server</span>"
    );
    $wus = BoincWorkunit::enum("batch = $batch->id");
    foreach($wus as $wu) {
        $resultid = $wu->canonical_resultid;
        if ($resultid) {
            $x = "<a href=result.php?resultid=$resultid>$resultid</a>";
            $y = "completed";
        } else {
            $x = "---";
            $y = "in progress";
        }

        echo "<tr>
                <td><a href=submit.php?action=query_job&wuid=$wu->id>$wu->id</a></td>
                <td>$y</td>
                <td>$x</td>
            </tr>
        ";
    }
    end_table();
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

// show the details of a job, including links to see the output files
// 
function handle_query_job($user) {
    $wuid = get_int('wuid');

    page_head("Job $wuid");

    echo "<a href=workunit.php?wuid=$wuid>View workunit page</a>\n";

    // show input files
    //
    echo "<h2>Input files</h2>\n";
    $wu = BoincWorkunit::lookup_id($wuid);
    $x = "<in>".$wu->xml_doc."</in>";
    $x = simplexml_load_string($x);
    start_table();
    table_header("Logical name<br><span class=note>(click to view)</span>",
        "Size (bytes)", "MD5"
    );
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    foreach ($x->workunit->file_ref as $fr) {
        $pname = (string)$fr->file_name;
        $lname = (string)$fr->open_name;
        $dir = filename_hash($pname, $fanout);
        $path = "../../download/$dir/$pname";
        $md5 = md5_file($path);
        $s = stat($path);
        $size = $s['size'];
        table_row(
            "<a href=/download/$dir/$pname>$lname</a>",
            $size,
            $md5
        );
    }
    end_table();

    echo "<h2>Instances</h2>\n";
    start_table();
    table_header(
        "Instance ID<br><span class=note>click for result page</span>",
        "State", "Output files"
    );
    $results = BoincResult::enum("workunitid=$wuid");
    foreach($results as $result) {
        echo "<tr>
            <td><a href=result.php?resultid=$result->id>$result->id</a></td>
            <td>".state_string($result)."</td>
            <td>
";
        $i = 0;
        if ($result->server_state == 5) {
            $names = get_outfile_names($result);
            $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
            $i = 0;
            foreach ($names as $name) {
                $url = boinc_get_output_file_url($user, $result, $i++);
                $path = dir_hier_path($name, "../../upload", $fanout);
                $s = stat($path);
                $size = $s['size'];
                echo "<a href=$url>$size bytes</a>";
            }
            $i++;
        }
        echo "</td></tr>\n";
    }
    end_table();
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

function handle_abort_batch_confirm() {
    $batch_id = get_int('batch_id');
    page_head("Confirm abort batch");
    echo "
        Aborting a batch will cancel all unstarted jobs.
        Are you sure you want to do this?
        <p>
    ";
    show_button(
        "submit.php?action=abort_batch&batch_id=$batch_id",
        "Yes - abort batch"
    );
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

function handle_abort_batch() {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error("no such batch");
    if ($batch->user_id != $user->id) {
        error("not owner");
    }
    abort_batch($batch);
    page_head("Batch aborted");
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

function handle_retire_batch_confirm() {
    $batch_id = get_int('batch_id');
    page_head("Confirm retire batch");
    echo "
        Retiring a batch will remove all of its output files.
        Are you sure you want to do this?
        <p>
    ";
    show_button(
        "submit.php?action=retire_batch&batch_id=$batch_id",
        "Yes - retire batch"
    );
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

function handle_retire_batch() {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if ($batch->user_id != $user->id) {
        error_page("not owner");
    }
    retire_batch($batch);
    page_head("Batch retired");
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

$user = get_logged_in_user();

$action = get_str('action', true);

switch ($action) {
case '': handle_main($user); break;
case 'abort_batch': handle_abort_batch(); break;
case 'abort_batch_confirm': handle_abort_batch_confirm(); break;
case 'query_batch': handle_query_batch($user); break;
case 'query_job': handle_query_job($user); break;
case 'retire_batch': handle_retire_batch(); break;
case 'retire_batch_confirm': handle_retire_batch_confirm(); break;
default:
    error_page('no such action');
}

?>
