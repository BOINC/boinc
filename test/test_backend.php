#! /usr/local/bin/php
<?php
    // End to end test.  Tests make_work, feeder, scheduling server, client,
    // file_upload_handler, validator, assimilator, and file_deleter on a
    // large batch of workunits.  Confirms that credit is correctly granted
    // and that unneeded files are deleted

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 4;
    $work->delay_bound = 86400;
    array_push($work->input_files, "input");

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts
    $project->install_make_work($work, 100, 5);

    $host->log_flags = "log_flags.xml";
    $host->add_user($user,$project);
    $host->install();

    echo "adding work\n";

    $work->install($project);

    $project->start_servers();

    // Start by generating a batch of 100 results
    echo "Generating 100 results... ";
    while( $project->num_wus_left() < 100 ) sleep(1);
    echo "done.\n";

    // Stop the project, deinstall make_work, and install the normal backend components
    $project->stop();
    $project->deinstall_make_work();
    $project->install_assimilator($app);
    $project->install_file_delete();
    $project->install_validate($app, 5);
    $project->install_feeder();

    while (($pid=exec("pgrep -n make_work")) != null) sleep(1);

    // Restart the server
    $project->restart();
    $project->start_servers();

    // Run the client until there's no more work
    $host->run("-exit_when_idle -skip_cpu_benchmarks");

    sleep(5);

    // *** DO TESTS HERE
    $result->state = RESULT_STATE_DONE;
    $result->exit_status = 0;
    $project->check_results(101, $result);

    // Stop the server
    $project->stop();
?>
