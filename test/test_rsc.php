#! /usr/local/bin/php
<?php
    // test whether the scheduling server filters out
    // work units too big for client

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts

    $host->log_flags = "log_flags.xml";
    $host->add_project($project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->nresults = 1;
    $work->rsc_disk = 1000000000000;    // 1 TB
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start();
    $host->run("-exit_when_idle");
    $project->stop();

    $x = $project->num_results_done();
    if ($x != 0) echo "Test failed\n";
    if ($x == 0) echo "Test succeeded\n";
?>
