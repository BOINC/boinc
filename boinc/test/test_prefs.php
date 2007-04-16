#! /usr/local/bin/php
<?php
    // test global preferences

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("uc_slow");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();      // must install projects before adding to hosts

    $host->log_flags = "log_flags.xml";
    $host->add_project($project);
    $host->global_prefs = "laptop_prefs";
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "ucs_wu";
    $work->result_template = "uc_result";
    array_push($work->input_files, "small_input");
    $work->install($project);

    $project->start_feeder();

    echo "Now run the client manually; start and stop it a few times.\n";

    //$project->check_results(1, $result);
    //$project->compare_file("ucs_wu_0_0", "uc_small_correct_output");
?>
