#! /usr/local/bin/php
<?php
    // test the concat application
    //
    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("concat");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts

    $host->add_project($project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "concat_wu";
    $work->result_template = "concat_result";
    $work->nresults = 2;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start();
    $host->run("-exit_when_idle");
    $project->stop();

    $project->check_results_done();
    $project->compare_file("concat_wu_0_0", "concat_correct_output");
    $project->compare_file("concat_wu_1_0", "concat_correct_output");
?>
