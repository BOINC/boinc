#!/usr/local/bin/php -q
<?php {
    // $Id$
    // tests whether command-line arg passing works

    include_once("test.inc");

    test_msg("standard concat application");

    $project = new Project;
    $project->add_core_and_version();
    $project->add_app_and_version("concat");

    $user = new User();
    $host = new Host($user);

    $project->add_user($user);
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host->add_user($user,$project);
    $host->install();

    $work = new Work($app);
    $work->wu_template = "concat_wu";
    $work->result_template = "concat_result";
    $work->redundancy = 2;
    array_push($work->input_files, "input");
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    $host->run("-exit_when_idle -skip_cpu_benchmarks");
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $project->check_results(2, $result);
    $project->compare_file("concat_wu_0_0", "concat_correct_output");
    $project->compare_file("concat_wu_1_0", "concat_correct_output");

    test_done();
} ?>
