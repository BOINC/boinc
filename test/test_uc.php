#! /usr/local/bin/php
<?php
    // This tests whether the most basic mechanisms are working
    // Also whether stderr output is reported correctly

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);

    // the following is optional
    $app = new App("core client");
    $app_version = new App_Version($app);
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
    $work->nresults = 2;
    $work->delay_bound = 10;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_feeder();
    $host->run("-exit_when_idle");
    $project->stop();

    $result->state = RESULT_STATE_DONE;
    $result->stderr_out = "APP: upper_case: starting, argc 1";
    $result->exit_status = 0;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");
?>
