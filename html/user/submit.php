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

define("PAGE_SIZE", 20);

function state_count($batches, $state) {
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state == $state) $n++;
    }
    return $n;
}

function show_all_link($batches, $state, $limit, $user, $app) {
    $n = state_count($batches, $state);
    if ($n > $limit) {
        if ($user) $userid = $user->id;
        else $userid = 0;
        if ($app) $appid = $app->id;
        else $appid = 0;

        echo "Showing the most recent $limit of $n batches.
            <a href=submit.php?action=show_all&state=$state&userid=$userid&appid=$appid>Show all $n</a>
        ";
    }
}

function show_in_progress($batches, $limit, $user, $app) {
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_IN_PROGRESS) continue;
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            echo "<h2>Batches in progress</h2>\n";
            if ($limit) {
                show_all_link($batches, BATCH_STATE_IN_PROGRESS, $limit, $user, $app);
            }
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
}

function show_complete($batches, $limit, $user, $app) {
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_COMPLETE) continue;
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            echo "<h2>Completed batches</h2>\n";
            if ($limit) {
                show_all_link($batches, BATCH_STATE_COMPLETE, $limit, $user, $app);
            }
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
    if ($first) {
        echo "<p>No completed batches.\n";
    } else {
        end_table();
    }
}

function show_aborted($batches, $limit, $user, $app) {
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_ABORTED) continue;
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            echo "<h2>Aborted batches</h2>\n";
            if ($limit) {
                show_all_link($batches, BATCH_STATE_ABORTED, $limit, $user, $app);
            }
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

// fill in the app and user names in list of batches
//
function fill_in_app_and_user_names(&$batches) {
    foreach ($batches as $batch) {
        //if ($batch->state < BATCH_STATE_COMPLETE || $batch->fraction_done < 1) {
        //    $wus = BoincWorkunit::enum("batch = $batch->id");
        //    $batch = get_batch_params($batch, $wus);
        //}
        $app = BoincApp::lookup_id($batch->app_id);
        if ($app) {
            $batch->app_name = $app->name;
        } else {
            $batch->app_name = "unknown";
        }
        $user = BoincUser::lookup_id($batch->user_id);
        if ($user) {
            $batch->user_name = $user->name;
        } else {
            $batch->user_name = "missing user $batch->user_id";
        }
    }
}

// show a set of batches
//
function show_batches($batches, $limit, $user, $app) {
    fill_in_app_and_user_names($batches);
    show_in_progress($batches, $limit, $user, $app);
    show_complete($batches, $limit, $user, $app);
    show_aborted($batches, $limit, $user, $app);
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
    show_batches($batches, PAGE_SIZE, $user, null);

    page_tail();
}

function check_admin_access($user, $app_id) {
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) error_page("no access");
    if ($app_id) {
        if (!$user_submit->manage_all) {
            $usa = BoincUserSubmitApp::lookup("user_id = $user->id and app_id=$app_id");
            if (!$usa) error_page("no access");
        }
    } else {
        if (!$user_submit->manage_all) error_page("no access");
    }
}

function handle_admin($user) {
    $app_id = get_int("app_id");
    check_admin_access($user, $app_id);
    if ($app_id) {
        $app = BoincApp::lookup_id($app_id);
        if (!$app) error_page("no such app");
        page_head("Administer $app->user_friendly_name");
        $batches = BoincBatch::enum("app_id = $app_id order by id desc");
        show_batches($batches, PAGE_SIZE, null, $app);
    } else {
        page_head("Administer all apps");
        $batches = BoincBatch::enum("true order by id desc");
        show_batches($batches, PAGE_SIZE, null, null);
    }
    page_tail();
}

// show the details of an existing batch
//
function handle_query_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    $app = BoincApp::lookup_id($batch->app_id);
    $wus = BoincWorkunit::enum("batch = $batch->id");
    $batch = get_batch_params($batch, $wus);

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
    $wu = BoincWorkunit::lookup_id($wuid);
    if (!$wu) error_page("no such job");

    page_head("Job $wuid");

    echo "
        <a href=workunit.php?wuid=$wuid>Workunit details</a> &middot;
        <a href=submit.php?action=query_batch?batch_id=$wu->batch>Batch $wu->batch</a>
    ";

    // show input files
    //
    echo "<h2>Input files</h2>\n";
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
                $upload_dir = parse_config(get_config(), "<upload_dir>");
                $path = dir_hier_path($name, $upload_dir, $fanout);
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

function show_batches_in_state($batches, $state) {
    switch ($state) {
    case BATCH_STATE_IN_PROGRESS:
        page_head("Batches in progress");
        show_in_progress($batches, 0, null, null);
        break;
    case BATCH_STATE_COMPLETE:
        page_head("Completed batches");
        show_complete($batches, 0, null, null);
        break;
    case BATCH_STATE_ABORTED:
        page_head("Aborted batches");
        show_aborted($batches, 0, null, null);
        break;
    }
    page_tail();
}

function handle_show_all($user) {
    $userid = get_int("userid");
    $appid = get_int("appid");
    $state = get_int("state");
    if ($userid) {
        // user looking at their own batches
        //
        if ($userid != $user->id) error_page("wrong user");
        $batches = BoincBatch::enum("user_id = $user->id and state=$state order by id desc");
        fill_in_app_and_user_names($batches);
        show_batches_in_state($batches, $state);
    } else {
        // admin looking at batches
        //
        check_admin_access($user, $appid);
        if ($appid) {
            $app = BoincApp::lookup_id($app_id);
            if (!$app) error_page("no such app");
            $batches = BoincBatch::enum("app_id = $app_id and state=$state order by id desc");
        } else {
            $batches = BoincBatch::enum("state=$state order by id desc");
        }
        fill_in_app_and_user_names($batches);
        show_batches_in_state($batches, $state);
    }
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
case 'show_all': handle_show_all($user); break;
default:
    error_page('no such action');
}

?>
