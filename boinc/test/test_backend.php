#!/usr/local/bin/php -q
<?php {
    // $Id$

    // End to end test.  Tests make_work, feeder, scheduling server, client,
    // file_upload_handler, validator, assimilator, timeout_check, and
    // file_deleter on a large batch of workunits.  Confirms that credit
    // is correctly granted and that unneeded files are deleted

    include_once("test.inc");

    test_msg("backend");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 5;
    $work->delay_bound = 70;
    array_push($work->input_files, "input");

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts
    $project->install_make_work($work, 499, 5);

    $host->log_flags = "log_flags.xml";
    $host->add_user($user,$project);
    $host->install();

    $work->install($project);

    $project->start_servers();

    // Start by generating a batch of 500 results
    $n = 0;
    while($n < 500 ) {
        $n = $project->num_wus_left();
        verbose_echo(1, "Generating results [$n/500]");
        sleep(1);
    }
    verbose_echo(1, "Generating results... 500 done");

    // Stop the project, deinstall make_work, and install the normal backend components
    $project->stop();
    $project->deinstall_make_work();
    $project->install_assimilator($app);
    $project->install_file_delete();
    $project->install_validate($app, 5);
    $project->install_feeder();
    $project->install_timeout_check($app, 5, 5, 0);

    while (($pid=exec("pgrep -n make_work")) != null) sleep(1);

    // Restart the server
    $project->restart();
    $project->start_servers();

    // Run the client until there's no more work
    $host->run("-exit_when_idle -skip_cpu_benchmarks");

    // Give the server 30 seconds to finish assimilating/deleting
    sleep(30);

    // *** DO CHECKS HERE
    $result->server_state = RESULT_SERVER_STATE_OVER;
    $result->exit_status = 0;
    $project->check_results(500, $result);

    // Stop the server
    $project->stop();

    test_done();
} ?>
