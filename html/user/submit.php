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

// show a set of batches
//
function show_batches($batches) {
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
        $user = BoincUser::lookup_id($batch->user_id);
        $batch->user_name = $user->name;
    }

    $first = true;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_IN_PROGRESS) continue;
        if ($first) {
            $first = false;
            echo "<h2>Batches in progress</h2>\n";
            start_table();
            table_header("name", "ID", "user", "app", "# jobs", "progress", "submitted");
        }
        $pct_done = (int)($batch->fraction_done*100);
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            "$pct_done%",
            local_time_str($batch->create_time)
        );
    }
    if ($first) {
        echo "<p>No in-progress batches.\n";
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
            table_header("name", "ID", "user", "app", "# jobs", "fraction done", "submitted");
        }
        $pct_done = (int)($batch->fraction_done*100);
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            "$pct_done%",
            local_time_str($batch->create_time)
        );
    }
    if ($first) {
        echo "<p>No completed batches.\n";
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
            table_header("name", "ID", "user", "app", "# jobs", "submitted");
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time)
        );
    }
    if (!$first) {
        end_table();
    }
}

// the job submission "home page":
// show the user's in-progress and completed batches,
// and a button for creating a new batch
//
function handle_main($user) {
    global $submit_urls;
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) {
        error_page("Ask the project admins for permission to submit jobs");
    }

    page_head("Job submission and control");
    echo "<h2>Submit jobs</h2>
        <ul>
    ";
    $x = "";
    foreach ($submit_urls as $appname=>$submit_url) {
        $app = BoincApp::lookup("name='$appname'");
        if (!$app) error_page("bad submit_url name: $appname");
        $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
        if ($usa || $user_submit->submit_all) {
            echo "<li> <a href=$submit_url> $app->user_friendly_name </a>";
        }
        if ($usa && $usa->manage) {
            $x .= "<li> <a href=submit.php?action=admin&app_id=$app->id>$app->user_friendly_name</a>
            ";
        }
    }
    echo "</ul>";
    if ($user_submit->manage_all || $x) {
        echo "<h2>Administer applications</h2>
            <ul>
            $x
        ";
        if ($user_submit->manage_all) {
            echo "<li><a href=submit.php?action=admin&app_id=0>All applications</a>";
        }
    }
    echo "</ul>";

    $batches = BoincBatch::enum("user_id = $user->id order by id desc");
    show_batches($batches);

    page_tail();
}

function handle_admin($user) {
    $app_id = get_int("app_id");
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) error_page("no access");
    if ($app_id) {
        if (!$user_submit->manage_all) {
            $usa = BoincUserSubmitApp::lookup("user_id = $user->id and app_id=$app_id");
            if (!$usa) error_page("no access");
        }
        $app = BoincApp::lookup_id($app_id);
        if (!$app) error_page("no such app");
        page_head("Administer $app->user_friendly_name");
        $batches = BoincBatch::enum("app_id = $app_id order by id desc");
        show_batches($batches);
    } else {
        if (!$user_submit->manage_all) error_page("no access");
        page_head("Administer all apps");
        $batches = BoincBatch::enum("true order by id desc");
        show_batches($batches);
    }
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
    row2("Output File Size (MB)", number_format(batch_output_file_size($batch->id)/1e6,2));
    end_table();
    if (batch_output_file_size($batch->id) <= 1e8) {
        $url = boinc_get_output_files_url($user, $batch_id);
        show_button($url, "Get zipped output files");
    } else {
        echo "<br/>The output file size of this batch is too big, it will be uploaded by FTP<br/>";
    }
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
        "Job ID and name<br><span class=note>click for details or to get output files</span>",
        "status",
        "Canonical instance<br><span class=note>click to see result page on BOINC server</span>",
        "Download Results"
    );
    $wus = BoincWorkunit::enum("batch = $batch->id");
    foreach($wus as $wu) {
        $resultid = $wu->canonical_resultid;
        $durl = boinc_get_wu_output_files_url($user,$wu->id);
        if ($resultid) {
            $x = "<a href=result.php?resultid=$resultid>$resultid</a>";
            $y = '<font color="green">completed</font>';
            $text = "<a href=$durl> Download Result Files</a>";
        } else {
            $x = "---";
            $text = "---";
            if ($batch->state == BATCH_STATE_COMPLETE) {
                $y = '<font color="red">failed</font>';
            }   else {
                $y = "in progress";
            }
        }
        echo "<tr>
                <td><a href=submit.php?action=query_job&wuid=$wu->id>$wu->id &middot; $wu->name</a></td>
                <td>$y</td>
                <td>$x</td>
                <td>$text</td>
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
        "State", "Output files<br><span class=note>click to view the file</span>"
    );
    $results = BoincResult::enum("workunitid=$wuid");
    foreach($results as $result) {
        echo "<tr>
            <td><a href=result.php?resultid=$result->id>$result->id | $result->name </a></td>
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
                echo "<a href=$url>$name </a> (".number_format($size)." bytes)<br/>";
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

function check_access($user, $batch) {
    if ($user->id == $batch->user_id) return;
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if ($user_submit->manage_all) return;
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$batch->app_id");
    if ($usa->manage) return;
    error_page("no access");
}

function handle_abort_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error_page("no such batch");
    check_access($user, $batch);
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

function handle_retire_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error_page("no such batch");
    check_access($user, $batch);
    retire_batch($batch);
    page_head("Batch retired");
    echo "<p><a href=submit.php>Return to job control page</a>\n";
    page_tail();
}

$user = get_logged_in_user();

$action = get_str('action', true);

switch ($action) {
case '': handle_main($user); break;
case 'abort_batch': handle_abort_batch($user); break;
case 'abort_batch_confirm': handle_abort_batch_confirm(); break;
case 'admin': handle_admin($user); break;
case 'query_batch': handle_query_batch($user); break;
case 'query_job': handle_query_job($user); break;
case 'retire_batch': handle_retire_batch($user); break;
case 'retire_batch_confirm': handle_retire_batch_confirm(); break;
default:
    error_page('no such action');
}

?>
