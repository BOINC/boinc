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
// Notes:
// - You'll need to adapt/extend this considerably;
//   e.g. the project URL, app name and user authenticator are hardwired here.
// - This can run on any host, not just the project server
//   (that's the "remote" part).
// - For convenience, this uses some functions from BOINC
//   (page_head() etc.).
//   When you adapt this to your own purposes,
//   you can strip out this stuff if the web site doesn't use BOINC

require_once("../inc/submit.inc");
require_once("../inc/util.inc");

error_reporting(E_ALL);
ini_set('display_errors', true);
ini_set('display_startup_errors', true);

$project = "http://isaac.ssl.berkeley.edu/test/";
$auth = "157f96a018b0b2f2b466e2ce3c7f54db";
$app_name = "uppercase";

function handle_main() {
    global $project, $auth, $app_name;
    $req->project = $project;
    $req->authenticator = $auth;
    list($batches, $errmsg) = boinc_query_batches($req);
    if ($errmsg) error_page($errmsg);

    page_head("Job submission and control");

    echo "
        <a href=submit_example.php?action=create_form><b>Create new batch</b></a>
    ";

    echo "<h2>Batches in progress</h2>\n";
    start_table();
    table_header("ID", "# jobs", "progress", "submitted");
    foreach ($batches as $batch) {
        if ($batch->completed) continue;
        $pct_done = (int)($batch->fraction_done*100);
        table_row(
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->njobs,
            "$pct_done%",
            time_str($batch->create_time)
        );
    }
    end_table();

    echo "<h2>Batches completed</h2>\n";
    start_table();
    table_header("ID", "# jobs", "submitted");
    foreach ($batches as $batch) {
        if (!$batch->completed) continue;
        table_row(
            "<a href=submit_example.php?action=query_batch&batch_id=$batch->id>$batch->id</a>",
            $batch->njobs,
            time_str($batch->create_time)
        );
    }
    end_table();

    page_tail();
}

function handle_create_form() {
    global $project, $auth, $app_name;

    page_head("Create batch");
    echo "
        <form action=submit_example.php>
        <input type=hidden name=action value=create_action>
    ";
    start_table();
    row2("Input file URL", "<input name=input_url size=60>");
    row2("Parameter low value", "<input name=param_lo>");
    row2("Parameter high value", "<input name=param_hi>");
    row2("Parameter increment", "<input name=param_inc>");
    row2("",
        "<input type=submit name=get_estimate value=\"Get completion time estimate\">"
    );
    row2("",
        "<input type=submit name=submit value=Submit>"
    );
    end_table();
    echo "</form>\n";
    page_tail();
}

// build a request object for boinc_*_batch() from form variables
//
function form_to_request() {
    global $project, $auth, $app_name;

    $input_url = get_str('input_url');
    if (!$input_url) error_page("missing input URL");
    $param_lo = get_str('param_lo');
    if (!$param_lo) error_page("missing param lo");
    $param_hi = get_str('param_hi');
    if (!$param_hi) error_page("missing param hi");
    $param_inc = get_str('param_inc');
    if (!$param_inc) error_page("missing param inc");

    $req->project = $project;
    $req->authenticator = $auth;
    $req->app_name = $app_name;
    $req->jobs = Array();

    $f->source = $input_url;
    $f->name = "in";
    $job->input_files = Array($f);

    for ($x=(double)$param_lo; $x<(double)$param_hi; $x += (double)$param_inc) {
        $job->rsc_fpops_est = $x*1e9;
        $job->command_line = "--t $x";
        $req->jobs[] = $job;
    }

    return $req;
}

function handle_create_action() {
    global $project, $auth, $app_name;

    $get_estimate = get_str('get_estimate', true);
    if ($get_estimate) {
        $req = form_to_request($project, $auth);
        list($e, $errmsg) = boinc_estimate_batch($req);
        if ($errmsg) error_page($errmsg);
        page_head("Batch estimate");
        echo "Estimate: $e seconds";
        page_tail();
    } else {
        $req = form_to_request($project, $auth);
        list($id, $errmsg) = boinc_submit_batch($req);
        if ($errmsg) error_page($errmsg);
        page_head("Batch submitted");
        echo "Batch ID: $id";
        page_tail();
    }
}

function handle_query_batch() {
    global $project, $auth, $app_name;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->batch_id = get_int('batch_id');
    list($reply, $errmsg) = boinc_query_batch($req);
    if ($errmsg) error_page($errmsg);

    page_head("Batch $req->batch_id");
    $url = boinc_get_output_files($req);
    echo "<a href=$url>Get zipped output files</a>\n";
    start_table();
    table_header("Job ID", "Canonical instance");
    foreach($reply->jobs as $job) {
        $id = (int)$job->id;
        $resultid = (int)$job->canonical_instance_id;
        if ($resultid) {
            $x = "<a href=result.php?resultid=$resultid>$resultid</a>";
        } else {
            $x = "---";
        }
        echo "<tr>
                <td><a href=submit_example.php?action=query_job&job_id=$id>$id</a></td>
                <td>$x</td>
            </tr>
        ";
    }
    end_table();
    page_tail();
}

function handle_query_job() {
    global $project, $auth, $app_name;
    $req->project = $project;
    $req->authenticator = $auth;
    $req->job_id = get_int('job_id');
    list($reply, $errmsg) = boinc_query_job($req);
    if ($errmsg) error_page($errmsg);

    page_head("Job $req->job_id");
    start_table();
    table_header("Instance ID", "State", "Output files");
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
    page_tail();
}

$action = get_str('action', true);

switch ($action) {
case '':
    handle_main();
    break;
case 'create_form':
    handle_create_form();
    break;
case 'create_action':
    handle_create_action();
    break;
case 'query_batch':
    handle_query_batch();
    break;
case 'query_job':
    handle_query_job();
    break;
default:
    error_page('no such action');
}

?>
