#! /usr/local/bin/php

<?php
    // test the client checkpoint/restart mechanism,

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
    $work->wu_template = "ucs_wu";
    $work->result_template = "ucs_result";
    $work->nresults = 1;
    array_push($work->input_files, "small_input");
    $work->install($project);

    $project->start_feeder();
    echo "Now run the client manually; start and stop it a few times\n";
    //compare_file("ucs_wu_0_0", "uc_small_correct_output");
?>
