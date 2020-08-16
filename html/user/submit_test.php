<?php
// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2018 University of California
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

// tests for remote job submission interfaces
//
// you must have a file "config.txt":
// line 0: project URL
// line 1: authenticator
//
// you must run this in a dir with a copy of or link to html/inc/submit.inc

// TODO: add more tests

require_once("submit.inc");

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

    for ($i=10; $i<20; $i++) {
        $job = new StdClass;
        $job->input_files = array($f);
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
