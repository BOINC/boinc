#!/usr/local/bin/php -q
<?php {
    // $Id$

    // test whether the scheduling server filters out work units too big for
    // client

    include_once("test.inc");

    test_msg("resource filtering for large work units");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $project->add_app_and_version("upper_case");

    $project->add_user($user);
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host->add_user($user,$project);
    $host->install();

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 1;
    $work->rsc_disk = 1000000000000;    // 1 TB
    $work->rsc_fpops = 0;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    $host->run("-exit_when_idle -skip_cpu_benchmarks");
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_UNSENT;
    $project->check_results(1, $result);

    test_done();
} ?>
