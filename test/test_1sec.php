#! /usr/local/bin/php
<?php
    // test the 1sec application
    // This tests whether CPU time is divided correctly between projects
    // TODO: make this test what it's supposed to test

    include_once("test.inc");

    $project1 = new Project;
    $project2 = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $project1->add_user($user);
    $project1->add_app($app);
    $project1->add_app_version($app_version);
    $project1->install();      // must install projects before adding to hosts

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

    $project1->check_results_done();
    $project1->compare_file("uc_wu_0_0", "uc_correct_output");
    $project1->compare_file("uc_wu_1_0", "uc_correct_output");
    $project2->check_results_done();
    $project2->compare_file("uc_wu_0_0", "uc_correct_output");
    $project2->compare_file("uc_wu_1_0", "uc_correct_output");
?>
