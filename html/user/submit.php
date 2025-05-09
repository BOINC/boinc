<?php

// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2024 University of California
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

// remote job submission functions.
//
// A 'remote app' is one where jobs are submitted remotely:
// - via web RPCs
// - (and possibly also) via forms on the project web site
//
// In both cases only users with permission can submit; either
// - a user_submit record with submit_all set
// - a user_submit_app record
//
// They are apps are described in $remote_apps in project.inc
//
// This page has several functions:
// - links to app-specific job-submission pages
// - Admin (if privileged user)
// - manage batches
//      view status, get output files, abort, retire
//   (this also shows batches created on the server)
// - set 'use only my computers'

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");
require_once("../project/project.inc");
require_once('../project/remote_apps.inc');

display_errors();

define("PAGE_SIZE", 20);

function return_link() {
    echo "<p><a href=submit.php?action=status>Return to job status page</a>\n";
}

// get params of in-progress batches; they might not be in progress anymore.
//
function get_batches_params($batches) {
    $b = [];
    foreach ($batches as $batch) {
        if ($batch->state == BATCH_STATE_IN_PROGRESS) {
            $wus = BoincWorkunit::enum_fields(
                'id, name, rsc_fpops_est, canonical_credit, canonical_resultid, error_mask',
                "batch = $batch->id"
            );
            $b[] = get_batch_params($batch, $wus);
        } else {
            $b[] = $batch;
        }
    }
    return $b;
}

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

// show in-progress batches.
//
function show_in_progress($batches, $limit, $user, $app) {
    echo "<h3>Batches in progress</h3>\n";
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state != BATCH_STATE_IN_PROGRESS) continue;
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            if ($limit) {
                show_all_link(
                    $batches, BATCH_STATE_IN_PROGRESS, $limit, $user, $app
                );
            }
            form_start('submit.php');
            form_input_hidden('action', 'abort_selected');
            start_table('table-striped');
            $x = [
                "Name",
                "ID",
                "User",
                "App",
                "# jobs",
                "Progress",
                "Submitted",
                //"Logical end time<br><small>Determines priority</small>"
            ];
            row_heading_array($x);
        }
        $pct_done = (int)($batch->fraction_done*100);
        $x = [
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            "$pct_done%",
            local_time_str($batch->create_time),
            //local_time_str($batch->logical_end_time)
        ];
        row_array($x);
    }
    if ($first) {
        echo "<p>None.\n";
    } else {
        end_table();
        form_end();
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
            echo "<h3>Completed batches</h3>\n";
            if ($limit) {
                show_all_link($batches, BATCH_STATE_COMPLETE, $limit, $user, $app);
            }
            form_start('submit.php', 'get');
            form_input_hidden('action', 'retire_multi');
            start_table('table-striped');
            table_header(
                "Name", "ID", "User", "App", "# Jobs", "Submitted", "Select"
            );
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time),
            sprintf('<input type=checkbox name=retire_%d>', $batch->id)
        );
    }
    if ($first) {
        echo "<p>No completed batches.\n";
    } else {
        end_table();
        form_submit('Retire selected batches');
        form_end();
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
            form_start('');
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
        form_end();
    }
}

// fill in the app and user names in list of batches
// TODO: speed this up by making list of app and user IDs
// and doing lookup just once.
//
function fill_in_app_and_user_names(&$batches) {
    $apps = [];
    foreach ($batches as $batch) {
        if (array_key_exists($batch->app_id, $apps)) {
            $app = $apps[$batch->app_id];
        } else {
            $app = BoincApp::lookup_id($batch->app_id);
            $apps[$batch->app_id] = $app;
        }
        if ($app) {
            $batch->app_name = $app->name;
            if ($batch->description) {
                $batch->app_name .= ": $batch->description";
            }
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
    $batches = get_batches_params($batches);
    show_in_progress($batches, $limit, $user, $app);
    show_complete($batches, $limit, $user, $app);
    show_aborted($batches, $limit, $user, $app);
}

// show links to per-app job submission forms
//
function handle_main($user) {
    global $remote_apps;
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) {
        error_page("Ask the project admins for permission to submit jobs");
    }

    page_head("Submit jobs");

    // show links to per-app job submission pages
    //
    foreach ($remote_apps as $area => $apps) {
        panel($area,
            function() use ($apps) {
                foreach ($apps as $app) {
                    if (empty($app->form)) continue;
                    // show app logo if available
                    if (!empty($app->logo)) {
                        echo sprintf(
                            '<a href=%s><img width=100 src=%s></a>&nbsp;',
                            $app->form, $app->logo
                        );
                    } else {
                        echo sprintf(
                            '<li><a href=%s>%s</a><p>',
                            $app->form, $app->long_name
                        );
                    }
                }
            }
        );
    }

    form_start('submit.php');
    form_input_hidden('action', 'update_only_own');
    form_radio_buttons(
        'Jobs you submit can run', 'only_own',
        [
            [0, 'on any computer'],
            [1, 'only on your computers']
        ],
        $user->seti_id
    );
    form_submit('Update');
    form_end();
    page_tail();
}

// show batches of logged in user.
// They have manage access to these batches.
//
function handle_show_status($user) {
    page_head("Job status");
    $batches = BoincBatch::enum("user_id = $user->id order by id desc");
    get_batches_params($batches);
    show_batches($batches, PAGE_SIZE, $user, null);

    page_tail();
}

function handle_update_only_own($user) {
    $val = get_int('only_own');
    $user->update("seti_id=$val");
    header("Location: submit.php");
}

// get list of app names of remote apps
//
function get_remote_app_names() {
    global $remote_apps;
    $x = [];
    foreach ($remote_apps as $category => $apps) {
        foreach ($apps as $app) {
            $x[] = $app->app_name;
        }
    }
    return array_unique($x);
}

// show links for everything the user has admin access to
//
function handle_admin($user) {
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) error_page('no access');
    if ($user_submit->manage_all) {
        // user can administer all apps
        //
        page_head("Job submission: manage all apps");
        echo "<li> <a href=submit.php?action=admin_all>View/manage all batches</a>
        ";
        $app_names = get_remote_app_names();
        foreach ($app_names as $app_name) {
            $app_name = BoincDb::escape_string($app_name);
            $app = BoincApp::lookup("name='$app_name'");
            echo "
                <li>$app->user_friendly_name<br>
                <ul>
                <li><a href=submit.php?action=admin_app&app_id=$app->id>View/manage batches</a>
            ";
            if ($app_name == 'buda') {
                echo "
                    <li> <a href=buda.php>Manage BUDA apps and variants</a>
                ";
            } else {
                echo "
                    <li> <a href=manage_app.php?app_id=$app->id&amp;action=app_version_form>Manage app versions</a>
                ";
            }
            echo "
                </ul>
            ";
        }
    } else {
        // see if user can administer specific apps
        //
        page_head("Job submission: manage apps");
        $usas = BoincUserSubmitApp::enum("user_id=$user->id");
        foreach ($usas as $usa) {
            $app = BoincApp::lookup_id($usa->app_id);
            echo "<li>$app->user_friendly_name<br>
                <a href=submit.php?action=admin_app&app_id=$app->id>Batches</a>
            ";
            if ($usa->manage) {
                echo "&middot;
                    <a href=manage_app.php?app_id=$app->id&action=app_version_form>Versions</a>
                ";
            }
        }
    }
    echo "</ul>\n";
    page_tail();
}

function handle_admin_app($user) {
    $app_id = get_int("app_id");
    $app = BoincApp::lookup_id($app_id);
    if (!$app) error_page("no such app");
    if (!has_manage_access($user, $app_id)) {
        error_page('no access');
    }

    page_head("Manage batches for $app->user_friendly_name");
    $batches = BoincBatch::enum("app_id = $app_id order by id desc");
    show_batches($batches, PAGE_SIZE, null, $app);
    page_tail();
}
function handle_admin_all($user) {
    page_head("Administer batches (all apps)");
    $batches = BoincBatch::enum("true order by id desc");
    show_batches($batches, PAGE_SIZE, null, null);
    page_tail();
}


// show the statics of mem/disk usage of jobs in a batch
//
function handle_batch_stats($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    $results = BoincResult::enum_fields(
        'peak_working_set_size, peak_swap_size, peak_disk_usage',
        sprintf('batch = %d and outcome=%d',
            $batch->id, RESULT_OUTCOME_SUCCESS
        )
    );
    page_head("Statistics for batch $batch_id");
    $n = 0;
    $wss_sum = 0;
    $swap_sum = 0;
    $disk_sum = 0;
    $wss_max = 0;
    $swap_max = 0;
    $disk_max = 0;
    foreach ($results as $r) {
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
    text_start(800);
    start_table('table-striped');
    row2("qualifying results", $n);
    row2("mean WSS", size_string($wss_sum/$n));
    row2("max WSS", size_string($wss_max));
    row2("mean swap", size_string($swap_sum/$n));
    row2("max swap", size_string($swap_max));
    row2("mean disk usage", size_string($disk_sum/$n));
    row2("max disk usage", size_string($disk_max));
    end_table();
    text_end();
    page_tail();
}

// return HTML for a color-coded batch progress bar
// green: successfully completed jobs
// red: failed
// light green: in progress
// light gray: unsent
//
function progress_bar($batch, $wus, $width) {
    $nsuccess = $batch->njobs_success;
    $nerror = $batch->nerror_jobs;
    $nin_prog = $batch->njobs_in_prog;
    $nunsent = $batch->njobs - $nsuccess - $nerror - $nin_prog;
    $w_success = $width*$nsuccess/$batch->njobs;
    $w_fail = $width*$nerror/$batch->njobs;
    $w_prog = $width*$nin_prog/$batch->njobs;
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
        $x .= "<td width=$w_unsent bgcolor=gray></td>";
    }
    $x .= "</tr></table>
        <strong>
        <font color=red>$nerror failed</font> &middot;
        <font color=green>$nsuccess completed</font> &middot;
        <font color=lightgreen>$nin_prog in progress</font> &middot;
        <font color=gray>$nunsent unsent</font>
        </strong>
    ";
    return $x;
}

// show the details of an existing batch.
// $user has access to abort/retire the batch
// and to get its output files
//
function handle_query_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    $app = BoincApp::lookup_id($batch->app_id);
    $wus = BoincWorkunit::enum_fields(
        'id, name, rsc_fpops_est, canonical_credit, canonical_resultid, error_mask',
        "batch = $batch->id"
    );
    $batch = get_batch_params($batch, $wus);
    if ($batch->user_id == $user->id) {
        $owner = $user;
    } else {
        $owner = BoincUser::lookup_id($batch->user_id);
    }

    $is_assim_move = is_assim_move($app);

    page_head("Batch $batch_id");
    text_start(800);
    start_table();
    row2("name", $batch->name);
    if ($batch->description) {
        row2('description', $batch->description);
    }
    if ($owner) {
        row2('submitter', $owner->name);
    }
    row2("application", $app?$app->name:'---');
    row2("state", batch_state_string($batch->state));
    //row2("# jobs", $batch->njobs);
    //row2("# error jobs", $batch->nerror_jobs);
    //row2("logical end time", time_str($batch->logical_end_time));
    if ($batch->expire_time) {
        row2("expiration time", time_str($batch->expire_time));
    }
    if ($batch->njobs) {
        row2("progress", progress_bar($batch, $wus, 600));
    }
    if ($batch->completion_time) {
        row2("completed", local_time_str($batch->completion_time));
    }
    row2("GFLOP/hours, estimated", number_format(credit_to_gflop_hours($batch->credit_estimate), 2));
    row2("GFLOP/hours, actual", number_format(credit_to_gflop_hours($batch->credit_canonical), 2));
    if (!$is_assim_move) {
        row2("Total size of output files",
            size_string(batch_output_file_size($batch->id))
        );
    }
    end_table();
    echo "<p>";

    if ($is_assim_move) {
        $url = "get_output3.php?action=get_batch&batch_id=$batch->id";
    } else {
        $url = "get_output2.php?cmd=batch&batch_id=$batch->id";
    }
    echo "<p>";
    show_button($url, "Get zipped output files");
    echo "<p>";
    switch ($batch->state) {
    case BATCH_STATE_IN_PROGRESS:
        show_button(
            "submit.php?action=abort_batch&batch_id=$batch_id",
            "Abort batch"
        );
        break;
    case BATCH_STATE_COMPLETE:
    case BATCH_STATE_ABORTED:
        show_button(
            "submit.php?action=retire_batch&batch_id=$batch_id",
            "Retire batch"
        );
        break;
    }
    echo "<p>";
    show_button("submit.php?action=batch_stats&batch_id=$batch_id",
        "Show memory/disk usage statistics"
    );

    echo "<h2>Jobs</h2>\n";
    start_table('table-striped');
    $x = [
        "Name <br><small>click for details</small>",
        'Status',
        'Credit'
    ];
    row_heading_array($x);
    foreach($wus as $wu) {
        $y = '';
        switch($wu->status) {
        case WU_SUCCESS:
            $resultid = $wu->canonical_resultid;
            $y = '<font color="green">completed</font>';
            break;
        case WU_ERROR:
            $y = '<font color="red">failed</font>';
            break;
        case WU_IN_PROGRESS:
            $y = '<font color="lightgreen">in progress</font>';
            break;
        case WU_UNSENT:
            $y = '<font color="gray">unsent</font>';
            break;
        }
        $x = [
            "<a href=submit.php?action=query_job&wuid=$wu->id>$wu->name</a>",
            $y,
            $wu->canonical_credit?number_format($wu->canonical_credit, 2):'---'
        ];
        row_array($x);
    }
    end_table();
    return_link();
    text_end();
    page_tail();
}

// Does the assimilator for the given app move output files
// to a results/<batchid>/ directory?
// This info is stored in the $remote_apps data structure in project.inc
//
function is_assim_move($app) {
    global $remote_apps;
    foreach ($remote_apps as $category => $apps) {
        foreach ($apps as $web_app) {
            if ($web_app->app_name == $app->name) {
                return $web_app->is_assim_move;
            }
        }
    }
    return false;
}

// show the details of a job, including links to see the output files
//
function handle_query_job($user) {
    $wuid = get_int('wuid');
    $wu = BoincWorkunit::lookup_id($wuid);
    if (!$wu) error_page("no such job");

    $app = BoincApp::lookup_id($wu->appid);
    $is_assim_move = is_assim_move($app);

    page_head("Job '$wu->name'");
    text_start(800);

    echo "
        <li><a href=workunit.php?wuid=$wuid>Workunit details</a>
        <p>
        <li><a href=submit.php?action=query_batch&batch_id=$wu->batch>Batch details</a>
    ";
    $d = "<foo>$wu->xml_doc</foo>";
    $x = simplexml_load_string($d);
    $x = $x->workunit;
    //echo "foo: $x->command_line";

    echo "<h2>Job instances</h2>\n";
    start_table('table-striped');
    table_header(
        "ID<br><small>click for details and stderr</small>",
        "State",
        "Output files"
    );
    $results = BoincResult::enum("workunitid=$wuid");
    $upload_dir = parse_config(get_config(), "<upload_dir>");
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    foreach($results as $result) {
        $x = [
            "<a href=result.php?resultid=$result->id>$result->id</a>",
            state_string($result)
        ];
        $i = 0;
        if ($result->server_state == RESULT_SERVER_STATE_OVER) {
            $phys_names = get_outfile_phys_names($result);
            $log_names = get_outfile_log_names($result);
            for ($i=0; $i<count($phys_names); $i++) {
                if ($is_assim_move) {
                    // file is in
                    // project/results/<batchid>/<wu_name>__file_<log_name>
                    $path = sprintf('results/%s/%s__file_%s',
                        $wu->batch, $wu->name, $log_names[$i]
                    );
                    $name = $log_names[$i];
                    $x[] = "$name: <a href=get_output3.php?action=get_file&path=$path>view</a> &middot; <a href=get_output3.php?action=get_file&path=$path&download=1>download</a>";
                } else {
                    $path = dir_hier_path(
                        $phys_names[$i], $upload_dir, $fanout
                    );
                    if (file_exists($path)) {
                        $url = sprintf(
                            'get_output2.php?cmd=result&result_id=%d&file_num=%d',
                            $result->id, $i
                        );
                        $s = stat($path);
                        $size = $s['size'];
                        $x[] = sprintf('<a href=%s>%s</a> (%s bytes)<br/>',
                            $url,
                            $log_names[$i],
                            number_format($size)
                        );
                    } else {
                        $x[] = sprintf("file '%s' is missing", $log_names[$i]);
                    }
                }
            }
        } else {
            $x[] = '---';
        }
        row_array($x);
    }
    end_table();

    // show input files
    //
    echo "<h2>Input files</h2>\n";
    $x = "<in>".$wu->xml_doc."</in>";
    $x = simplexml_load_string($x);
    start_table('table-striped');
    table_header("Name<br><small>(click to view)</small>", "Size (bytes)");
    foreach ($x->workunit->file_ref as $fr) {
        $pname = (string)$fr->file_name;
        $lname = (string)$fr->open_name;
        foreach ($x->file_info as $fi) {
            if ((string)$fi->name == $pname) {
                table_row(
                    "<a href=$fi->url>$lname</a>",
                    $fi->nbytes
                );
                break;
            }
        }
    }

    end_table();
    text_end();
    return_link();
    page_tail();
}

// is user allowed to retire or abort this batch?
//
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

    if (get_int('confirmed', true)) {
        abort_batch($batch);
        page_head("Batch aborted");
        return_link();
        page_tail();
    } else {
        page_head("Confirm abort batch");
        echo "
            Aborting a batch will cancel all unstarted jobs.
            Are you sure you want to do this?
            <p>
        ";
        show_button(
            "submit.php?action=abort_batch&batch_id=$batch_id&confirmed=1",
            "Yes - abort batch"
        );
        return_link();
        page_tail();
    }
}

function handle_retire_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error_page("no such batch");
    check_access($user, $batch);

    if (get_int('confirmed', true)) {
        retire_batch($batch);
        page_head("Batch retired");
        return_link();
        page_tail();
    } else {
        page_head("Confirm retire batch");
        echo "
            Retiring a batch will remove all of its output files.
            Are you sure you want to do this?
            <p>
        ";
        show_button(
            "submit.php?action=retire_batch&batch_id=$batch_id&confirmed=1",
            "Yes - retire batch"
        );
        return_link();
        page_tail();
    }
}

function handle_retire_multi($user) {
    $batches = BoincBatch::enum(
        sprintf('user_id=%d and state=%d', $user->id, BATCH_STATE_COMPLETE)
    );
    page_head('Retiring batches');
    foreach ($batches as $batch) {
        $x = sprintf('retire_%d', $batch->id);
        if (get_str($x, true) == 'on') {
            retire_batch($batch);
            echo "<p>retired batch $batch->name\n";
        }
    }
    return_link();
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
        if (!has_manage_access($user, $appid)) {
            error_page('no access');
        }
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
case 'admin': handle_admin($user); break;
case 'admin_app': handle_admin_app($user); break;
case 'admin_all': handle_admin_all($user); break;
case 'batch_stats': handle_batch_stats($user); break;
case 'query_batch': handle_query_batch($user); break;
case 'query_job': handle_query_job($user); break;
case 'retire_batch': handle_retire_batch($user); break;
case 'retire_multi': handle_retire_multi($user); break;
case 'show_all': handle_show_all($user); break;
case 'status': handle_show_status($user); break;
case 'update_only_own': handle_update_only_own($user); break;
default:
    error_page("no such action $action");
}

?>
