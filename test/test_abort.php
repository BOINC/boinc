#!/usr/local/bin/php -q
<?php {
    // This tests whether the result abort mechanism is working

    include_once("test.inc");

    test_msg("result abort mechanism (disk space limit)");

    $project = new Project;
    $project->add_app_and_version("upper_case");

    $user = new User();

    $project->add_user($user);
    $project->install();      // must install projects before adding to hosts
    $project->install_feeder();

    $host = new Host();
    $host->add_user($user, $project);
    $host->install();

    $work = new Work();
    $work->wu_template = "uc_wu";
    $work->result_template = "abort_result";
    $work->redundancy = 2;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_servers();
    $host->run("-exit_when_idle -skip_cpu_benchmarks -sched_retry_delay_min 1 >& /dev/null");

    $project->validate(2);
    $result->server_state = RESULT_SERVER_STATE_OVER;
    $project->check_results(2, $result);

    $project->assimilate();
    $project->file_delete();

    $project->stop();

    test_done();
} ?>
