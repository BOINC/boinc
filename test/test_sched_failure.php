#! /usr/local/bin/php
<?php
//This tests the exponential backoff mechanism on the client in case of scheduling server failures.
//This test is not automated. It has to be run, and then client.out (in the host directory) must be looked at to examine wether everything is working correctly.    
    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);

    $app = new App("upper_case");
    $app_version = new App_Version($app);

    // the following is optional (makes client web download possible)
    $core_app = new App("core client");
    $core_app_version = new App_Version($core_app);
    $project->add_app($core_app);
    $project->add_app_version($core_app_version);

    $project->add_user($user);
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
    $work->redundancy = 2;
    $work->delay_bound = 10;
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_feeder();
    $project->delete_scheduler();
    $pid = $host->run_asynch("-exit_when_idle");
    echo "sleeping for 100 seconds\n";
    sleep(100);
    $project->reinstall_scheduler();
    $status = 0;
    //wait until the host has stopped running
    pcntl_waitpid($pid,$status,0);
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $result->stderr_out = "APP: upper_case: starting, argc 1";
    $result->exit_status = 0;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");
?>
