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
// - via forms on the project web site
//
// In both cases only users with permission can submit; either
// - a user_submit record with submit_all set
// - a user_submit_app record
//
// These apps are described in $remote_apps in project.inc
//
// This page provides several functions involving remote apps
// ('me' means logged-in user)
//
//  show_all_batches
//      show list of batches visible to me, possibly filtered by
//          submitting user
//          app
//          state (in progress, completed etc.)
//  show_user_batches
//      show my batches
//  show_batches_admin_app
//      show all batches for a given app to admin
//  admin_all
//      show all batches to admin
//  batch_stats
//      show WSS, disk usage stats for a batch
//  query_batch
//      show list of jobs in a batch
//  query_job
//      show job details and instances
//  retire_batch
//      retire a batch
//  retire_multi
//      retire multiple batches
//  abort_batch
//      abort a batch
//  admin
//      show index of admin functions
//  update_only_own
//      control whether my jobs should run only on my computers

require_once("../inc/submit_db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");
require_once("../inc/submit_util.inc");
require_once("../project/project.inc");
require_once('../project/remote_apps.inc');

display_errors();

// in general, if there can be lots of something,
// show this many and a link to show all.
// TODO: jobs in a batch
//
define("PAGE_SIZE", 20);

function return_link() {
    echo "<p><a href=submit.php?action=show_user_batches>Return to batches page</a>\n";
}

// return subset of batches in given state
//
function batches_in_state($all_batches, $state) {
    $batches = [];
    foreach ($all_batches as $batch) {
        if ($batch->state != $state) continue;
        $batches[] = $batch;
    }
    return $batches;
}

function sort_batches(&$batches, $order) {
    switch ($order) {
    case 'sub_asc':
        $f = function($a, $b) {
            return (int)($a->create_time - $b->create_time);
        };
        break;
    case 'sub_desc':
        $f = function($a, $b) {
            return (int)($b->create_time - $a->create_time);
        };
        break;
    case 'comp_asc':
        $f = function($a, $b) {
            return (int)($a->completion_time - $b->completion_time);
        };
        break;
    case 'comp_desc':
        $f = function($a, $b) {
            return (int)($b->completion_time - $a->completion_time);
        };
        break;
    }
    usort($batches, $f);
}

// in progress batches don't have completion time
//
function in_progress_order($order) {
    switch ($order) {
    case 'comp_asc': return 'sub_asc';
    case 'comp_desc': return 'sub_desc';
    }
    return $order;
}

// show order options
// sub_asc, sub_desc: submission time ( = ID order)
// comp_asc, comp_desc: completion time

function order_options($url_args, $order) {
    $url = "submit.php?$url_args";
    echo sprintf(
        'Order by: submission time (%s, %s) or completion time (%s, %s)',
        order_item($url, $order, 'sub_asc', 'ascending'),
        order_item($url, $order, 'sub_desc', 'descending'),
        order_item($url, $order, 'comp_asc', 'ascending'),
        order_item($url, $order, 'comp_desc', 'descending')
    );
}

function order_item($url, $cur_order, $order, $label) {
    if ($cur_order == $order) {
        return $label;
    } else {
        $url .= "&order=$order";
        return "<a href=$url>$label</a>";
    }
}

function get_order() {
    $order = get_str('order', true);
    if (!$order) $order = 'sub_desc';
    return $order;
}

function state_count($batches, $state) {
    $n = 0;
    foreach ($batches as $batch) {
        if ($batch->state == $state) $n++;
    }
    return $n;
}

function show_all_batches_link($batches, $state, $limit, $user, $app) {
    $n = state_count($batches, $state);
    if ($n > $limit) {
        if ($user) $userid = $user->id;
        else $userid = 0;
        if ($app) $appid = $app->id;
        else $appid = 0;

        echo "Showing the most recent $limit of $n batches.
            <a href=submit.php?action=show_all_batches&state=$state&userid=$userid&appid=$appid>Show all $n</a>
            <p>
        ";
    }
}

// show in-progress batches.
//
function show_in_progress($all_batches, $order, $limit, $user, $app) {
    $batches = batches_in_state($all_batches, BATCH_STATE_IN_PROGRESS);
    sort_batches($batches, in_progress_order($order));
    echo sprintf('<h3>Batches in progress (%d)</h3>', count($batches));
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            if ($limit) {
                show_all_batches_link(
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
                "Submitted"
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
            local_time_str($batch->create_time)
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

function show_complete($all_batches, $order, $limit, $user, $app) {
    $batches = batches_in_state($all_batches, BATCH_STATE_COMPLETE);
    sort_batches($batches, $order);
    echo sprintf('<h3>Completed batches (%d)</h3>', count($batches));
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            if ($limit) {
                show_all_batches_link($batches, BATCH_STATE_COMPLETE, $limit, $user, $app);
            }
            form_start('submit.php', 'get');
            form_input_hidden('action', 'retire_multi');
            start_table('table-striped');
            table_header(
                "Name", "ID", "User", "App", "# Jobs", "Submitted", "Completed", "Select"
            );
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time),
            local_time_str($batch->completion_time),
            sprintf('<input type=checkbox name=retire_%d>', $batch->id)
        );
    }
    if ($first) {
        echo "<p>None.\n";
    } else {
        end_table();
        form_submit('Retire selected batches');
        form_end();
    }
}

function show_aborted($all_batches, $order, $limit, $user, $app) {
    $batches = batches_in_state($all_batches, BATCH_STATE_ABORTED);
    if (!$batches) return;
    sort_batches($batches, $order);
    echo sprintf('<h3>Aborted batches (%d)</h3>', count($batches));
    $first = true;
    $n = 0;
    foreach ($batches as $batch) {
        if ($limit && $n == $limit) break;
        $n++;
        if ($first) {
            $first = false;
            if ($limit) {
                show_all_batches_link($batches, BATCH_STATE_ABORTED, $limit, $user, $app);
            }
            form_start('submit.php', 'get');
            form_input_hidden('action', 'retire_multi');
            start_table();
            table_header("Name", "ID", "User", "App", "# Jobs", "Submitted", "Aborted", 'Select');
        }
        table_row(
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->user_name,
            $batch->app_name,
            $batch->njobs,
            local_time_str($batch->create_time),
            local_time_str($batch->completion_time),
            sprintf('<input type=checkbox name=retire_%d>', $batch->id)
        );
    }
    if (!$first) {
        end_table();
        form_submit('Retire selected batches');
        form_end();
    }
}

// fill in the app and user names in list of batches
//
function fill_in_app_and_user_names(&$batches) {
    $apps = [];
    $users = [];
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
                $batch->app_name .= " ($batch->description)";
            }
        } else {
            $batch->app_name = "unknown";
        }

        if (array_key_exists($batch->user_id, $users)) {
            $user = $users[$batch->user_id];
        } else {
            $user = BoincUser::lookup_id($batch->user_id);
            $users[$batch->user_id] = $user;
        }
        if ($user) {
            $batch->user_name = $user->name;
        } else {
            $batch->user_name = "missing user $batch->user_id";
        }
    }
}

// show a set of batches: in progress, then completed, then aborted
//
function show_batches($batches, $order, $limit, $user, $app) {
    fill_in_app_and_user_names($batches);
    $batches = get_batches_params($batches);
    show_in_progress($batches, $order, $limit, $user, $app);
    show_complete($batches, $order, $limit, $user, $app);
    show_aborted($batches, $order, $limit, $user, $app);
}

// show links to per-app job submission forms
//
function show_submit_links($user) {
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
                            '<a href=%s>%s</a><p>',
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
            [1, 'only on your computers (and no other jobs will run there)']
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
function handle_show_user_batches($user) {
    page_head("Your batches");
    $order = get_order();
    order_options('action=show_user_batches', $order);
    $batches = BoincBatch::enum("user_id = $user->id");
    show_batches($batches, $order, PAGE_SIZE, $user, null);

    page_tail();
}

function handle_update_only_own($user) {
    if (!parse_bool(get_config(), 'enable_assignment')) {
        error_page(
            'Job assignment is not enabled in the project config file.
            Please ask the project admins to enable it.'
        );
        return;
    }
    $val = get_int('only_own');
    if ($val) {
        if (BoincHost::count("userid=$user->id") == 0) {
            error_page(
                "You don't have any computers running BOINC and attached to this project."
            );
        }
    }
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
        echo '<ul>';
        echo "<li> <a href=submit.php?action=admin_all>View/manage all batches</a>
        ";
        $app_names = get_remote_app_names();
        foreach ($app_names as $app_name) {
            $app_name = BoincDb::escape_string($app_name);
            $app = BoincApp::lookup("name='$app_name'");
            echo "
                <li>$app->user_friendly_name<br>
                <ul>
                <li><a href=submit.php?action=show_batches_admin_app&app_id=$app->id>View/manage batches</a>
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
        echo '<ul>';
        $usas = BoincUserSubmitApp::enum("user_id=$user->id");
        foreach ($usas as $usa) {
            $app = BoincApp::lookup_id($usa->app_id);
            echo "<li>$app->user_friendly_name<br>
                <a href=submit.php?action=show_batches_admin_app&app_id=$app->id>Batches</a>
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

// show all batches for given app to administrator
//
function show_batches_admin_app($user) {
    $app_id = get_int("app_id");
    $app = BoincApp::lookup_id($app_id);
    if (!$app) error_page("no such app");
    if (!has_manage_access($user, $app_id)) {
        error_page('no access');
    }

    $order = get_order();

    page_head("Manage batches for $app->user_friendly_name");
    order_options("action=show_batches_admin_app&app_id=$app_id", $order);
    $batches = BoincBatch::enum("app_id = $app_id");
    show_batches($batches, $order, PAGE_SIZE, null, $app);
    page_tail();
}

function handle_admin_all($user) {
    $order = get_order();
    page_head("Administer batches (all apps and users)");
    order_options("action=admin_all", $order);
    $batches = BoincBatch::enum('');
    show_batches($batches, $order, PAGE_SIZE, null, null);
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

define('COLOR_SUCCESS', 'green');
define('COLOR_FAIL', 'red');
define('COLOR_IN_PROGRESS', 'deepskyblue');
define('COLOR_UNSENT', 'gray');

// return HTML for a color-coded batch progress bar
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
        $x .= sprintf('<td width=%d bgcolor=%s></td>', $w_fail, COLOR_FAIL);
    }
    if ($w_success) {
        $x .= sprintf('<td width=%d bgcolor=%s></td>', $w_success, COLOR_SUCCESS);
    }
    if ($w_prog) {
        $x .= sprintf('<td width=%d bgcolor=%s></td>', $w_prog, COLOR_IN_PROGRESS);
    }
    if ($w_unsent) {
        $x .= sprintf('<td width=%d bgcolor=%s></td>', $w_unsent, COLOR_UNSENT);
    }
    $x .= sprintf('</tr></table>
        <strong>
        <font color=%s>%d failed</font> &middot;
        <font color=%s>%d completed</font> &middot;
        <font color=%s>%d in progress</font> &middot;
        <font color=%s>%d unsent</font>
        </strong>',
        COLOR_FAIL, $nerror,
        COLOR_SUCCESS, $nsuccess,
        COLOR_IN_PROGRESS, $nin_prog,
        COLOR_UNSENT, $nunsent
    );
    return $x;
}

// show the details of an existing batch.
// $user has access to abort/retire the batch
// and to get its output files
//
function handle_query_batch($user) {
    $batch_id = get_int('batch_id');
    $status = get_int('status', true);
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error_page('no batch');
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
    row2("Batch name", $batch->name);
    if ($batch->description) {
        row2('Description', $batch->description);
    }
    if ($owner) {
        row2('Submitter',
            "<a href=show_user.php?userid=$owner->id>$owner->name</a>"
        );
    }
    row2("Submitted", time_str($batch->create_time));
    row2("Application", $app?$app->name:'---');
    row2("State", batch_state_string($batch->state));
    //row2("# jobs", $batch->njobs);
    //row2("# error jobs", $batch->nerror_jobs);
    //row2("logical end time", time_str($batch->logical_end_time));
    if ($batch->expire_time) {
        row2("Median turnaround time", time_diff($batch->expire_time));
    }
    if ($batch->njobs) {
        row2('Progress', progress_bar($batch, $wus, 600));
    }
    if ($batch->completion_time) {
        row2("Completed", local_time_str($batch->completion_time));
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
    echo "<p>
        <h3>Completed jobs</h3>
        <ul>
        <li>
        <a href=submit_stats.php?action=graphs&batch_id=$batch_id>Job runtimes</a>
        <li>
        <a href=submit.php?action=batch_stats&batch_id=$batch_id>Memory/disk usage</a>
        <li>
        <a href=submit_stats.php?action=show_hosts&batch_id=$batch_id>Grouped by host</a>
        </ul>
        <h3>Failed jobs</h3>
        <ul>
        <li>
        <a href=submit_stats.php?action=err_host&batch_id=$batch_id>Grouped by host</a>
        <li>
        <a href=submit_stats.php?action=err_code&batch_id=$batch_id>Grouped by exit code</a>
        </ul>
    ";

    echo "<h2>Jobs</h2>\n";
    $url = "submit.php?action=query_batch&batch_id=$batch_id";
    echo "Show: ";
    echo sprintf('
        <a href=%s&status=%d>failed</a> &middot;
        <a href=%s&status=%d>completed</a> &middot;
        <a href=%s&status=%d>in progress</a> &middot;
        <a href=%s&status=%d>unsent</a> &middot;
        <a href=%s>all</a>
        <p>',
        $url, WU_ERROR,
        $url, WU_SUCCESS,
        $url, WU_IN_PROGRESS,
        $url, WU_UNSENT,
        $url
    );

    start_table('table-striped');
    $x = [
        "Name <br><small>click for details</small>",
        "status",
        "GFLOPS-hours"
    ];
    row_heading_array($x);
    foreach($wus as $wu) {
        if ($status && $wu->status != $status) continue;
        $y = '';
        $c = '---';
        switch($wu->status) {
        case WU_SUCCESS:
            $resultid = $wu->canonical_resultid;
            $y = sprintf('<font color="%s">completed</font>', COLOR_SUCCESS);
            $c = number_format(
                credit_to_gflop_hours($wu->canonical_credit), 2
            );
            break;
        case WU_ERROR:
            $y = sprintf('<font color="%s">failed</font>', COLOR_FAIL);
            break;
        case WU_IN_PROGRESS:
            $y = sprintf('<font color="%s">in progress</font>', COLOR_IN_PROGRESS);
            break;
        case WU_UNSENT:
            $y = sprintf('<font color="%s">unsent</font>', COLOR_UNSENT);
            break;
        }
        $x = [
            "<a href=submit.php?action=query_job&wuid=$wu->id>$wu->name</a>",
            $y,
            $c
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

    echo "
        <ul>
        <li><a href=workunit.php?wuid=$wuid>Job details</a>
        <p>
        <li><a href=submit.php?action=query_batch&batch_id=$wu->batch>Batch details</a>
        </ul>
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
        'Sent',
        'Received',
        'Priority',
        "Output files"
    );
    $results = BoincResult::enum("workunitid=$wuid");
    $upload_dir = parse_config(get_config(), "<upload_dir>");
    $fanout = parse_config(get_config(), "<uldl_dir_fanout>");
    foreach ($results as $result) {
        $x = [
            "<a href=result.php?resultid=$result->id>$result->id</a>",
            state_string($result),
            time_str($result->sent_time),
            time_str($result->received_time),
            $result->priority
        ];
        if ($is_assim_move) {
            if ($result->id == $wu->canonical_resultid) {
                $log_names = get_outfile_log_names($result);
                $nfiles = count($log_names);
                for ($i=0; $i<$nfiles; $i++) {
                    $name = $log_names[$i];
                    // don't show 'view' link if it's a .zip
                    $y = "$name: ";
                    if (!strstr($name, '.zip')) {
                        $y .= sprintf(
                            '<a href=get_output3.php?action=get_file&result_id=%d&index=%d>view</a> &middot; ',
                            $result->id, $i
                        );
                    }
                    $y .= sprintf(
                        '<a href=get_output3.php?action=get_file&result_id=%d&index=%d&download=1>download</a>',
                        $result->id, $i
                    );
                    $x[] = $y;
                }
            } else {
                $x[] = '---';
            }
        } else {
            if ($result->server_state == RESULT_SERVER_STATE_OVER) {
                $phys_names = get_outfile_phys_names($result);
                $log_names = get_outfile_log_names($result);
                $nfiles = count($log_names);
                for ($i=0; $i<$nfiles; $i++) {
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
            } else {
                $x[] = '---';
            }
        }
        row_array($x);
    }
    end_table();

    // show input files
    //
    echo "<h2>Input files</h2>\n";
    $x = "<in>".$wu->xml_doc."</in>";
    $x = simplexml_load_string($x);
    if ($x->workunit->file_ref) {
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
    } else {
        echo "The job has no input files.<p>";
    }

    return_link();
    page_tail();
}

// is user allowed to retire or abort this batch?
//
function has_access($user, $batch) {
    if ($user->id == $batch->user_id) return true;
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if ($user_submit->manage_all) return true;
    $usa = BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$batch->app_id");
    if ($usa->manage) return true;
    return false;
}

function handle_abort_batch($user) {
    $batch_id = get_int('batch_id');
    $batch = BoincBatch::lookup_id($batch_id);
    if (!$batch) error_page("no such batch");
    if (!has_access($user, $batch)) {
        error_page("no access");
    }

    if (get_int('confirmed', true)) {
        abort_batch($batch);
        page_head("Batch $batch_id aborted");
        return_link();
        page_tail();
    } else {
        page_head("Confirm abort batch");
        echo "
            <p>
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
    if (!has_access($user, $batch)) {
        error_page("no access");
    }

    if (get_int('confirmed', true)) {
        retire_batch($batch);
        page_head("Batch $batch_id retired");
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

// retire multiple batches
//
function handle_retire_multi($user) {
    $batches = BoincBatch::enum(
        sprintf('state in (%d, %d)', BATCH_STATE_COMPLETE, BATCH_STATE_ABORTED)
    );
    page_head('Retiring batches');
    foreach ($batches as $batch) {
        if (!has_access($user, $batch)) {
            continue;
        }
        $x = sprintf('retire_%d', $batch->id);
        if (get_str($x, true) == 'on') {
            retire_batch($batch);
            echo "<p>retired batch $batch->id ($batch->name)\n";
        }
    }
    return_link();
    page_tail();
}

// given a list of batches, show the ones in a given state
//
function show_batches_in_state($batches, $state, $url_args, $order) {
    switch ($state) {
    case BATCH_STATE_IN_PROGRESS:
        page_head("Batches in progress");
        order_options($url_args, $order);
        show_in_progress($batches, $order, 0, null, null);
        break;
    case BATCH_STATE_COMPLETE:
        page_head("Completed batches");
        order_options($url_args, $order);
        show_complete($batches, $order, 0, null, null);
        break;
    case BATCH_STATE_ABORTED:
        page_head("Aborted batches");
        order_options($url_args, $order);
        show_aborted($batches, $order, 0, null, null);
        break;
    }
    page_tail();
}

// show all batches visible to user, possibly limited by user/app/state
function handle_show_all_batches($user) {
    $userid = get_int("userid");
    $appid = get_int("appid");
    $state = get_int("state");
    $order = get_order();
    $url_args = "action=show_all_batches&state=$state&userid=$userid&appid=$appid";
    if ($userid) {
        // user looking at their own batches
        //
        if ($userid != $user->id) error_page("wrong user");
        $batches = BoincBatch::enum("user_id=$user->id and state=$state");
        fill_in_app_and_user_names($batches);
        $batches = get_batches_params($batches);
        show_batches_in_state($batches, $state, $url_args, $order);
    } else {
        // admin looking at batches
        //
        if (!has_manage_access($user, $appid)) {
            error_page('no access');
        }
        if ($appid) {
            $app = BoincApp::lookup_id($appid);
            if (!$app) error_page("no such app");
            $batches = BoincBatch::enum("app_id=$appid and state=$state");
        } else {
            $batches = BoincBatch::enum("state=$state");
        }
        fill_in_app_and_user_names($batches);
        $batches = get_batches_params($batches);
        show_batches_in_state($batches, $state, $url_args, $order);
    }
}

$user = get_logged_in_user();

$action = get_str('action', true);

switch ($action) {

// links to job submission forms
case '': show_submit_links($user); break;

// show lists of batches
case 'show_all_batches': handle_show_all_batches($user); break;
case 'show_user_batches': handle_show_user_batches($user); break;
case 'show_batches_admin_app': show_batches_admin_app($user); break;
case 'admin_all': handle_admin_all($user); break;

// show info about a batch or job
case 'batch_stats': handle_batch_stats($user); break;
case 'query_batch': handle_query_batch($user); break;
case 'query_job': handle_query_job($user); break;

// operations on batches
case 'retire_batch': handle_retire_batch($user); break;
case 'retire_multi': handle_retire_multi($user); break;
case 'abort_batch': handle_abort_batch($user); break;

// access control
case 'admin': handle_admin($user); break;

// 'run jobs only on my computers' flag (stored in user.seti_id)
case 'update_only_own': handle_update_only_own($user); break;

default:
    error_page("no such action $action");
}

?>
