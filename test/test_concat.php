#! /usr/local/bin/php
<?php
    // tests whether command-line arg passing works

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
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_feeder();
    $host->run("-exit_when_idle");
    $project->stop();

    $result->state = RESULT_STATE_DONE;
    $project->check_results(2, $result);
    $project->compare_file("concat_wu_0_0", "concat_correct_output");
    $project->compare_file("concat_wu_1_0", "concat_correct_output");
?>
