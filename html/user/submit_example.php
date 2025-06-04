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

// example of a web interface to remote job submission
//
// Although the architecture is intended to support remote portals,
// this example must run on the BOINC project server.
// In particular:
// - It assumes cookie-based authentication,
//   as is done by the BOINC code.
//   You'll need to adapt/extend this considerably to run this
//   on a server other than the BOINC project server.
// - It uses some functions from BOINC (page_head() etc.).
//   When you adapt this to your own purposes,
//   you can strip out this stuff if the web site doesn't use BOINC

// THIS IS PRETTY MUCH USELESS

require_once("../inc/submit.inc");
require_once("../inc/common_defs.inc");
require_once("../inc/submit_db.inc");
require_once("../inc/submit_util.inc");
    // needed for access control stuff
require_once("../inc/util.inc");
require_once("../project/project.inc");

display_errors();

// hardwired app name for now
define('APP_NAME', 'remote_test');

$project = master_url();
$user = get_logged_in_user();
$auth = $user->authenticator;

// the job submission "home page":
// show the user's in-progress and completed batches,
// and a button for creating a new batch
//
function handle_main() {
    global $project, $auth;
    $req = new StdClass;
    $req->project = $project;
    $req->authenticator = $auth;
    list($batches, $errmsg) = boinc_query_batches($req);
    if ($errmsg) error_page(htmlentities($errmsg));

    page_head("Job submission and control");

    echo "
        This is an example of a web interface
        for remote submission of BOINC jobs.
        It lets you
        <ul>
        <li> Upload files
        <li> submit batches of jobs,
        <li> see the status of in-progress and completed batches.
        </ul>
        <p>
        To use this, you must be logged in as a user
        with permission to submit jobs.
        <p>
    ";
    show_button("submit_example.php?action=create_form", "Create new batch");

    $first = true;
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
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
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
            table_header("name", "ID", "# jobs", "submitted");
        }
        table_row(
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
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
            table_header("name", "ID", "# jobs", "submitted");
        }
        table_row(
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->name</a>",
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->njobs,
            local_time_str($batch->create_time)
        );
    }
    if (!$first) {
        end_table();
    }

    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

// return an array of the apps this user is allowed to submit jobs for
//
function eligible_apps() {
    global $user;
    $apps = BoincApp::enum("deprecated = 0");
    $user_submit = BoincUserSubmit::lookup_userid($user->id);
    if (!$user_submit) return null;
    $a = array();
    foreach($apps as $app) {
        if ($user_submit->submit_all) {
            $a[] = $app;
        } else {
            if (BoincUserSubmitApp::lookup("user_id=$user->id and app_id=$app->id")) {
                $a[] = $app;
            }
        }
    }
    return $a;
}

// return HTML for a popup menu of apps
//
function app_select($apps) {
    $x = '<select class="form-control" name="app_name">\n';
    foreach ($apps as $app) {
        $x .= "<option value=$app->name>$app->user_friendly_name</option>\n";
    }
    $x .= "</select>\n";
    return $x;
}

// show a form for creating a batch.
//
function handle_create_form() {
    global $project, $auth;

    $apps = eligible_apps();
    if (!$apps) error_page("You are not allowed to submit jobs");
    page_head("Create batch");
    echo "
        This form lets you specify a batch of jobs,
        and either submit it or get and estimate of its completion time.
        <p>
        The job runs an application that
        <ul>
        <li> Reads an input file, converts it to upper case,
          and writes this to an output file.
            You give a URL from which this file can be read.
        <li> Takes a command-line parameter, and uses that
          number of seconds of CPU time
        You can specify a range of values for the parameter;
        this determines the number of jobs in the batch.
        </ul>

        <p>
        <form action=submit_example.php>
        <input type=hidden name=action value=create_action>
    ";
    start_table();
    row2("Batch name", "<input name=batch_name value=\"enter name\">");
//    row2("Application", app_select($apps));
    row2("Input file URL", "<input name=input_url size=60 value=\"https://google.com/\">");
    row2("Parameter low value (0..60)", "<input name=param_lo value=10>");
    row2("Parameter high value (0..60)", "<input name=param_hi value=20>");
    row2("Parameter increment (>= 1)", "<input name=param_inc value=1>");
    row2("",
        "<input class=\"btn btn-default\" type=submit name=get_estimate value=\"Get completion time estimate\">"
    );
    row2("",
        "<input class=\"btn btn-primary\" type=submit name=submit value=Submit>"
    );
    end_table();
    echo "</form>\n";
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

// build a request object for boinc_*_batch() from form variables
//
function form_to_request() {
    global $project, $auth;

    $input_url = get_str('input_url');
    if (!$input_url) error_page("missing input URL");
    $param_lo = (double)get_str('param_lo');
    if ($param_lo<0 || $param_lo>60) error_page("param lo must be in 0..60");
    $param_hi = (double)get_str('param_hi');
    if ($param_hi<0 || $param_hi>60 || $param_hi <= $param_lo) {
        error_page("param hi must be in 0..60 and > param lo");
    }
    $param_inc = (double)get_str('param_inc');
    if ($param_inc < 1) error_page("param inc must be >= 1");

    $req = new StdClass;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->app_name = APP_NAME;
    $req->batch_name = get_str('batch_name');
    $req->jobs = array();

    $f = new StdClass;
    $f->source = $input_url;
    $f->mode = 'semilocal';

    for ($x=$param_lo; $x<$param_hi; $x += $param_inc) {
        $job = new StdClass;
        $job->rsc_fpops_est = $x*1e9;
        $job->command_line = "--t $x";
        $job->input_files = array($f);
        $req->jobs[] = $job;
    }

    return $req;
}

// create and submit a batch
//
function handle_create_action() {
    global $project, $auth;

    $get_estimate = get_str('get_estimate', true);
    if ($get_estimate) {
        $req = form_to_request($project, $auth);
        list($e, $errmsg) = boinc_estimate_batch($req);
        if ($errmsg) error_page(htmlentities($errmsg));
        page_head("Batch estimate");
        echo sprintf("Estimate: %.0f seconds", $e);
        echo "<p><a href=submit_example.php>Return to job control page</a>\n";
        page_tail();
    } else {
        $req = form_to_request($project, $auth);
        list($id, $errmsg) = boinc_submit_batch($req);
        if ($errmsg) error_page(htmlentities($errmsg));
        page_head("Batch submitted");
        echo "Batch created, ID: $id\n";
        echo "<p><a href=submit_example.php>Return to job control page</a>\n";
        page_tail();
    }
}

// show the details of an existing batch
//
function handle_query_batch() {
    global $project, $auth;
    $req = (object)array(
        'project' => $project,
        'authenticator' => $auth,
        'batch_id' => get_int('batch_id'),
    );
    list($batch, $errmsg) = boinc_query_batch($req);
    if ($errmsg) error_page(htmlentities($errmsg));

    page_head("Batch $req->batch_id");
    start_table();
    row2("name", $batch->name);
    row2("application", $batch->app_name);
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
    echo "<p>";
    $url = boinc_get_output_files($req);
    show_button($url, "Get zipped output files");
    echo "<p>";
    switch ($batch->state) {
    case BATCH_STATE_IN_PROGRESS:
        show_button(
            "submit_example.php?action=abort_batch_confirm&batch_id=$req->batch_id",
            "Abort batch"
        );
        break;
    case BATCH_STATE_COMPLETE:
    case BATCH_STATE_ABORTED:
        show_button(
            "submit_example.php?action=retire_batch_confirm&batch_id=$req->batch_id",
            "Retire batch"
        );
        break;
    }

    echo "<h2>Jobs</h2>\n";
    start_table();
    table_header(
        "Job ID<br><p class=\"text-muted\">click for details or to get output files</p>",
        "status",
        "Canonical instance<br><p class=\"text-muted\">click to see result page on BOINC server</p>"
    );
    foreach($batch->jobs as $job) {
        $id = (int)$job->id;
        $resultid = (int)$job->canonical_instance_id;
        if ($resultid) {
            $x = "<a href=result.php?resultid=$resultid>$resultid</a>";
            $y = "completed";
        } else {
            $x = "---";
            $y = "in progress";
        }

        echo "<tr>
                <td><a href=submit_example.php?action=query_job&job_id=$id>$id</a></td>
                <td>$y</td>
                <td>$x</td>
            </tr>
        ";
    }
    end_table();
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

// show the details of a job, including links to see the output files
//
function handle_query_job() {
    global $project, $auth;
    $req = new StdClass;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->job_id = get_int('job_id');

    list($reply, $errmsg) = boinc_query_job($req);
    if ($errmsg) error_page(htmlentities($errmsg));

    page_head("Job $req->job_id");
    echo "<a href=$project/workunit.php?wuid=$req->job_id>View workunit page on BOINC server</a>\n";
    echo "<h2>Instances</h2>\n";
    start_table();
    table_header(
        "Instance ID<br><p class=\"text-muted\">click for result page on BOINC server</p>",
        "State", "Output files"
    );
    foreach($reply->instances as $inst) {
        echo "<tr>
            <td><a href=result.php?resultid=$inst->id>$inst->id</a></td>
            <td>$inst->state</td>
            <td>
";
        $i = 0;
        foreach ($inst->outfiles as $outfile) {
            $req->instance_name = $inst->name;
            $req->file_num = $i;
            $url = boinc_get_output_file($req);
            echo "<a href=$url>$outfile->size bytes</a>";
            $i++;
        }
        echo "</td></tr>\n";
    }
    end_table();
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
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
        "submit_example.php?action=abort_batch&batch_id=$batch_id",
        "Yes - abort batch"
    );
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

function handle_abort_batch() {
    global $project, $auth;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->batch_id = get_int('batch_id');
    $errmsg = boinc_abort_batch($req);
    if ($errmsg) error_page(htmlentities($errmsg));
    page_head("Batch aborted");
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
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
        "submit_example.php?action=retire_batch&batch_id=$batch_id",
        "Yes - retire batch"
    );
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

function handle_retire_batch() {
    global $project, $auth;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->batch_id = get_int('batch_id');
    $errmsg = boinc_retire_batch($req);
    if ($errmsg) error_page(htmlentities($errmsg));
    page_head("Batch retired");
    echo "<p><a href=submit_example.php>Return to job control page</a>\n";
    page_tail();
}

$action = get_str('action', true);
switch ($action) {
case '': handle_main(); break;
case 'abort_batch': handle_abort_batch(); break;
case 'abort_batch_confirm': handle_abort_batch_confirm(); break;
case 'create_action': handle_create_action(); break;
case 'create_form': handle_create_form(); break;
case 'query_batch': handle_query_batch(); break;
case 'query_job': handle_query_job(); break;
case 'retire_batch': handle_retire_batch(); break;
case 'retire_batch_confirm': handle_retire_batch_confirm(); break;
default:
    error_page('no such action');
}

?>
