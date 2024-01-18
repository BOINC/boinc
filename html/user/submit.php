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

// web interface for managing batches
// (e.g. as part of remote job submission).
// Lets you see the status of batches, get their output files,
// abort them, retire them, etc.

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");
require_once("../project/project.inc");

display_errors();

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
            <p>
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
            table_header(
                "Name",
                "ID",
                "User",
                "App",
                "# jobs",
                "Progress",
                "Submitted",
                "Logical end time<br><small>Determines priority</small>"
            );
        }
        $pct_done = (int)($batch->fraction_done*100);
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            "$pct_done%",
            local_time_str($batch->create_time),
            local_time_str($batch->logical_end_time)
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

    if (isset($submit_urls)) {
        // show links to per-app job submission pages
        //
        echo "<h3>Submit jobs</h3>
            <ul>
        ";
        foreach ($submit_urls as $appname=>$submit_url) {
            $appname = BoincDb::escape_string($appname);
            $app = BoincApp::lookup("name='$appname'");
            if (!$app) error_page("bad submit_url name: $appname");
            $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id");
            if ($usa || $user_submit->submit_all) {
                echo "<li> <a href=$submit_url> $app->user_friendly_name </a>";
            }
        }
        echo "</ul>\n";
    }

    // show links to admin pages if relevant
    //
    $usas = BoincUserSubmitApp::enum("user_id=$user->id");
    $app_admin = false;
    foreach ($usas as $usa) {
        if ($usa->manage) {
            $app_admin = true;
            break;
        }
    }
    if ($user_submit->manage_all || $app_admin) {
        echo "<h3>Administrative functions</h3><ul>\n";
        if ($user_submit->manage_all) {
            echo "<li>All applications<br>
                <a href=submit.php?action=admin&app_id=0>Batches</a>
                &middot;
                <a href=manage_project.php>Users</a>
            ";
            $apps = BoincApp::enum("deprecated=0");
            foreach ($apps as $app) {
                echo "<li>$app->user_friendly_name<br>
                    <a href=submit.php?action=admin&app_id=$app->id>Batches</a>
                    &middot;
                    <a href=manage_app.php?app_id=$app->id>Manage</a>
                ";
            }
        } else {
            foreach ($usas as $usa) {
                $app = BoincApp::lookup_id($usa->app_id);
                echo "<li>$app->user_friendly_name<br>
                    <a href=submit.php?action=admin&app_id=$app->id>Batches</a>
                ";
                if ($usa->manage) {
                    echo "&middot;
                        <a href=manage_app.php?app_id=$app->id&action=app_version_form>Versions</a>
                    ";
                }
            }
        }
        echo "</ul>\n";
    }

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
        page_head("Administer batches for $app->user_friendly_name");
        $batches = BoincBatch::enum("app_id = $app_id order by id desc");
        show_batches($batches, PAGE_SIZE, null, $app);
    } else {
        page_head("Administer batches (all apps)");
        $batches = BoincBatch::enum("true order by id desc");
        show_batches($batches, PAGE_SIZE, null, null);
    }
    page_tail();
}


// show the statics of mem/disk usage of jobs in a batch
//
function handle_batch_stats($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    $results = BoincResult::enum("batch = $batch->id");
    page_head("Statistics for batch $batch_id");
    $n = 0;
    $wss_sum = 0;
    $swap_sum = 0;
    $disk_sum = 0;
    $wss_max = 0;
    $swap_max = 0;
    $disk_max = 0;
    foreach ($results as $r) {
        if ($r->outcome != RESULT_OUTCOME_SUCCESS) {
            continue;
        }
        // pre-7.3.16 clients don't report usage info
        //
        if ($r->peak_working_set_size == 0) {
            continue;
        }
        $n++;
        $wss_sum += $r->peak_working_set_size;
        if ($r->peak_working_set_size > $wss_max) {
            $wss_max = $r->peak_working_set_size;
        }
        $swap_sum += $r->peak_swap_size;
        if ($r->peak_swap_size > $swap_max) {
            $swap_max = $r->peak_swap_size;
        }
        $disk_sum += $r->peak_disk_usage;
        if ($r->peak_disk_usage > $disk_max) {
            $disk_max = $r->peak_disk_usage;
        }
    }
    if ($n == 0) {
        echo "No qualifying results.";
        page_tail();
        return;
    }
    start_table();
    row2("qualifying results", $n);
    row2("mean WSS", size_string($wss_sum/$n));
    row2("max WSS", size_string($wss_max));
    row2("mean swap", size_string($swap_sum/$n));
    row2("max swap", size_string($swap_max));
    row2("mean disk usage", size_string($disk_sum/$n));
    row2("max disk usage", size_string($disk_max));
    end_table();

    page_tail();
}

// return HTML for a color-coded batch progress bar
// green: successfully completed jobs
// red: failed
// light green: in progress
// light gray: unsent
//
function progress_bar($batch, $wus, $width) {
    $w_success = $width*$batch->fraction_done;
    $w_fail = $width*$batch->nerror_jobs/$batch->njobs;
    $nsuccess = $batch->njobs * $batch->fraction_done;
    $nsent = wus_nsent($wus);
    $nprog = $nsent - $nsuccess - $batch->nerror_jobs;
    $w_prog = $width*$nprog/$batch->njobs;
    $nunsent = $batch->njobs-$nsent;
    $w_unsent = $width*$nunsent/$batch->njobs;
    $x = '<table height=20><tr>';
    if ($w_fail) {
        $x .= "<td width=$w_fail bgcolor=red></td>";
    }
    if ($w_success) {
        $x .= "<td width=$w_success bgcolor=green></td>";
    }
    if ($w_prog) {
        $x .= "<td width=$w_prog bgcolor=lightgreen></td>";
    }
    if ($w_unsent) {
        $x .= "<td width=$w_unsent bgcolor=lightgray></td>";
    }
    $x .= "</tr></table>
        <strong>
        <font color=red>$batch->nerror_jobs fail</font> &middot;
        <font color=green>$nsuccess success</font> &middot;
        <font color=lightgreen>$nprog in progress</font> &middot;
        <font color=lightgray>$nunsent unsent</font>
        </strong>
    ";
    return $x;
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
    row2("application", $app?$app->name:'---');
    row2("state", batch_state_string($batch->state));
    //row2("# jobs", $batch->njobs);
    //row2("# error jobs", $batch->nerror_jobs);
    //row2("logical end time", time_str($batch->logical_end_time));
    if ($batch->expire_time) {
        row2("expiration time", time_str($batch->expire_time));
    }
    row2("progress", progress_bar($batch, $wus, 600));
    if ($batch->completion_time) {
        row2("completed", local_time_str($batch->completion_time));
    }
    row2("GFLOP/hours, estimated", number_format(credit_to_gflop_hours($batch->credit_estimate), 2));
    row2("GFLOP/hours, actual", number_format(credit_to_gflop_hours($batch->credit_canonical), 2));
    row2("Output File Size", size_string(batch_output_file_size($batch->id)));
    end_table();
    $url = "get_output2.php?cmd=batch&batch_id=$batch->id";
    show_button($url, "Get zipped output files");
    switch ($batch->state) {
    case BATCH_STATE_IN_PROGRESS:
        echo "<p></p>";
        show_button(
            "submit.php?action=abort_batch_confirm&batch_id=$batch_id",
            "Abort batch"
        );
        break;
    case BATCH_STATE_COMPLETE:
    case BATCH_STATE_ABORTED:
        echo "<p></p>";
        show_button(
            "submit.php?action=retire_batch_confirm&batch_id=$batch_id",
            "Retire batch"
        );
        break;
    }
    show_button("submit.php?action=batch_stats&batch_id=$batch_id",
        "Show memory/disk usage statistics"
    );

    echo "<h2>Jobs</h2>\n";
    start_table();
    table_header(
        "Job ID and name<br><small>click for details or to get output files</small>",
        "status",
        "Canonical instance<br><small>click to see result page on BOINC server</smallp>",
        "Download Results"
    );
    foreach($wus as $wu) {
        $resultid = $wu->canonical_resultid;
        if ($resultid) {
            $x = "<a href=result.php?resultid=$resultid>$resultid</a>";
            $y = '<font color="green">completed</font>';
            $text = "<a href=get_output2.php?cmd=workunit&wu_id=$wu->id>Download output files</a>";
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
        <a href=submit.php?action=query_batch&batch_id=$wu->batch>Batch $wu->batch</a>
    ";

    // show input files
    //
    echo "<h2>Input files</h2>\n";
    $x = "<in>".$wu->xml_doc."</in>";
    $x = simplexml_load_string($x);
    start_table();
    table_header("Logical name<br><small>(click to view)</small>",
        "Size (bytes)", "MD5"
    );
    foreach ($x->workunit->file_ref as $fr) {
        $pname = (string)$fr->file_name;
        $lname = (string)$fr->open_name;
        foreach ($x->file_info as $fi) {
            if ((string)$fi->name == $pname) {
                table_row(
                    "<a href=$fi->url>$lname</a>",
                    $fi->nbytes,
                    $fi->md5_cksum
                );
                break;
            }
        }
    }
    end_table();

    echo "<h2>Instances</h2>\n";
    start_table();
    table_header(
        "Instance ID<br><small>click for result page</small>",
        "State", "Output files<br><small>click to view the file</small>"
    );
    $results = BoincResult::enum("workunitid=$wuid");
    $upload_dir = parse_config(get_config(), "<upload_dir>");
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    foreach($results as $result) {
        echo "<tr>
            <td><a href=result.php?resultid=$result->id>$result->id &middot; $result->name </a></td>
            <td>".state_string($result)."</td>
            <td>
";
        $i = 0;
        if ($result->server_state == 5) {
            $phys_names = get_outfile_names($result);
            $log_names = get_outfile_log_names($result);
            for ($i=0; $i<count($phys_names); $i++) {
                $url = sprintf(
                    'get_output2.php?cmd=result&result_id=%d&file_num=%d',
                    $result->id, $i
                );
                $path = dir_hier_path($phys_names[$i], $upload_dir, $fanout);
                $s = stat($path);
                $size = $s['size'];
                echo sprintf('<a href=%s>%s</a> (%s bytes)<br/>',
                    $url,
                    $log_names[$i],
                    number_format($size)
                );
            }
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
            $app = BoincApp::lookup_id($appid);
            if (!$app) error_page("no such app");
            $batches = BoincBatch::enum("app_id = $appid and state=$state order by id desc");
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
case 'batch_stats': handle_batch_stats($user); break;
case 'query_batch': handle_query_batch($user); break;
case 'query_job': handle_query_job($user); break;
case 'retire_batch': handle_retire_batch($user); break;
case 'retire_batch_confirm': handle_retire_batch_confirm(); break;
case 'show_all': handle_show_all($user); break;
default:
    error_page("no such action $action");
}

?>
