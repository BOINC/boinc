#! /usr/local/bin/php
<?php
    // This tests whether the client handles multiple projects,
    // and whether CPU time is divided correctly between projects
    // The client should do work for project 2 5 times faster
    // than for project 1

    include_once("test.inc");

    $project1 = new Project;
    $project2 = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $project1->resource_share = 1;
    $project1->add_user($user);
    $project1->add_app($app);
    $project1->add_app_version($app_version);
    $project1->install();      // must install projects before adding to hosts

    $project2->resource_share = 5;
    $project2->add_user($user);
    $project2->add_app($app);
    $project2->add_app_version($app_version);
    $project2->install();      // must install projects before adding to hosts

    $host->add_project($project1);
    $host->add_project($project2);
    $host->install();

    echo "adding work\n";

    $work = new Work($project, $app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->nresults = 5;
    array_push($work->input_files, "input");
    $work->install($project1);
    $work->install($project2);

    $project1->start();
    $project2->start();
    $host->run("-exit_when_idle");
    $project1->stop();
    $project2->stop();

    $result->state = RESULT_STATE_DONE;
    $project1->check_results(2, $result);
    $project1->compare_file("uc_wu_0_0", "uc_correct_output");
    $project1->compare_file("uc_wu_1_0", "uc_correct_output");
    $project2->check_results(2, $result);
    $project2->compare_file("uc_wu_0_0", "uc_correct_output");
    $project2->compare_file("uc_wu_1_0", "uc_correct_output");
?>
