#! /usr/local/bin/php
<?php
// Tests the exponential backoff mechanism on the client for failed downloads
// This test is not automated. It has to be run, and then client.out (in the host directory) must be looked at to examine wether everything is working correctly.    
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
    $project->install_feeder();

    $host->log_flags = "log_flags.xml";
    $host->add_user($user, $project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 2;
    array_push($work->input_files, "input");
    $work->install($project);

    //delete the download_dir immediately 
    $project->delete_downloaddir();

    $project->start_servers();
    $pid = $host->run_asynch("-exit_when_idle");
    echo "sleeping 100 secs\n";
    sleep(100);
    $project->reinstall_downloaddir(null);
    $status = 0;
    //wait until the host has stopped running
    pcntl_waitpid($pid, $status, 0);
    $project->stop();

    $result->server_state = RESULT_SERVER_STATE_OVER;
    $result->stderr_out = "APP: upper_case: starting, argc 1";
    $result->exit_status = 0;
    $project->check_results(2, $result);
    $project->compare_file("uc_wu_0_0", "uc_correct_output");
    $project->compare_file("uc_wu_1_0", "uc_correct_output");
?>
