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

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 5;
    $work->delay_bound = 60;
    array_push($work->input_files, "input");

    $project1->resource_share = 1;
    $project1->shmem_key = "0x12344321";
    $project1->short_name = "Project1";
    $project1->long_name = "Project1";
    $project1->add_user($user);
    $project1->add_app($app);
    $project1->add_app_version($app_version);
    $project1->install();      // must install projects before adding to hosts
    $project1->install_feeder();

    $project2->resource_share = 5;
    $project2->shmem_key = "0x12345678";
    $project2->short_name = "Project2";
    $project2->long_name = "Project2";
    $project2->add_user($user);
    $project2->add_app($app);
    $project2->add_app_version($app_version);
    $project2->install();      // must install projects before adding to hosts
    $project2->install_feeder();

    $host->add_user($user,$project1);
    $host->add_user($user,$project2);
    $host->install();

    echo "adding work\n";

    $work->install($project1);
    $work->install($project2);

    $project1->start_servers();
    $project2->start_servers();
    $host->run("-exit_when_idle -no_time_test");
    $project1->stop();
    $project2->stop();

    $result->state = RESULT_STATE_DONE;
    $project1->check_results(5, $result);
    $project1->compare_file("uc_wu_0_0", "uc_correct_output");
    $project1->compare_file("uc_wu_1_0", "uc_correct_output");
    $project2->check_results(5, $result);
    $project2->compare_file("uc_wu_0_0", "uc_correct_output");
    $project2->compare_file("uc_wu_1_0", "uc_correct_output");
?>
