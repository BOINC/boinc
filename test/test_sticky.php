#! /usr/local/bin/php
<?php
    // test the sticky file mechanism

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
    $work->wu_template = "uc_wu_sticky";
    $work->result_template = "uc_result_sticky";
    $work->nresults = 2;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_feeder();
    $host->run("-exit_when_idle");
    $project->stop();

    $result->state = RESULT_STATE_DONE;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");

    // make sure result files are still there
    $host->check_file_present($project, "uc_wu_0_0");
    $host->check_file_present($project, "uc_wu_1_0");

?>
