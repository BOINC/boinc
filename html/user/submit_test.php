<?php
require_once("submit.inc");

// tests for remote job submission interfaces
//
// you must have a file "config.txt":
// line 0: project URL
// line 1: authenticator
//
// you must run this in a dir with a link to submit.inc

// TODO: add more tests

// for this test, you must have
// - an app "uppercase"
// - templates uppercase_in and uppercase_out
// - a staged file "input"
//
function test_submit_batch($req) {
    $req->app_name = "uppercase";
    $req->jobs = array();

    $f = new StdClass;
    $f->mode = "local_staged";
    $f->source = "input";

    $job = new StdClass;
    $job->input_files = array($f);

    for ($i=10; $i<20; $i++) {
        $job->rsc_fpops_est = $i*1e9;
        $job->command_line = "--t $i";
        $req->jobs[] = $job;
    }

    list($batch_id, $errmsg) = boinc_submit_batch($req);
    if ($errmsg) {
        echo "Error: $errmsg\n";
    } else {
        echo "Batch ID: $batch_id\n";
    }
}

function test_query_batch($req, $id) {
    $req->batch_id = $id;
    $req->get_cpu_time = true;
    list($r, $errmsg) = boinc_query_batch($req);
    if ($errmsg) {
        echo "Error: $errmsg\n";
    } else {
        print_r($r);
    }
}

$config = file("config.txt");
$project = trim($config[0]);
$auth = trim($config[1]);

$req = new StdClass;
$req->project = $project;
$req->authenticator = $auth;

//test_submit_batch($req);
test_query_batch($req, 267);
