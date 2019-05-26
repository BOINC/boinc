#! /usr/local/bin/php
<?php
    // This tests whether the most basic mechanisms are working
    // Also whether stderr output is reported correctly

    include_once("test.inc");

    $project = new Project;
    $user = new User();
    $host = new Host($user);
    $app = new App("upper_case");
    $app_version = new App_Version($app);

    $project->add_user($user);
    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->start_feeder = true;
    $project->start_make_work = true;
    $project->start_validate = true;
    $project->start_file_delete = true;
    $project->start_assimilator = true;
    $project->install();      // must install projects before adding to hosts

    $host->log_flags = "log_flags.xml";
    $host->add_user($user,$project);
    $host->install();

    echo "adding work\n";

    $work = new Work($app);
    $work->wu_template = "uc_wu";
    $work->result_template = "uc_result";
    $work->redundancy = 3;
  
    array_push($work->input_files, "input");
    $work->install($project);

    $project->start_feeder();
    $project->start_make_work($work);
    $project->start_validate($app, 3);
    $project->start_file_delete();
    $project->start_assimilator($app);
    $project->start_stripchart();
    //$project->start_servers();
    $host->run();
    $project->stop();
?>
