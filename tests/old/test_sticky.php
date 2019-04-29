#!/usr/local/bin/php -q
<?php {
    // $Id$

    include_once("test.inc");

    test_msg("sticky file mechanism");

    $project = new Project;
    $user = new User();
    $host = new Host($user);

    $project->add_user($user);
    $project->add_app_and_version("upper_case");
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host->add_user($user,$project);
    $host->install();

    $work = new Work($app);
    $work->wu_template = "uc_wu_sticky";
    $work->result_template = "uc_result_sticky";
    $work->redundancy = 2;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    $host->run("-exit_when_idle -skip_cpu_benchmarks");
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_sticky_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_sticky_1_0", "uc_correct_output");

    // make sure result files are still there
    $host->check_file_present($project, "uc_wu_sticky_0_0");
    $host->check_file_present($project, "uc_wu_sticky_1_0");

    test_done();
} ?>
