#! /usr/local/bin/php
<?php
    // This script creates a BOINC project.
    // You just need to plug in an application,
    // and back-end systems for creating work and validating results

    include_once("test.inc");

    $project = new Project;
    $project->name = "apt";

    $platform = new Platform("windows_intelx86", "Windows");

    $app = new App("Astropulse");
    $app_version = new App_Version($app);
    $app_version->platform = $platform;
    $app_version->exec_dir = "../apps";
    $app_version->exec_name = "ap_win_0.02.exe";

    $core_app = new App("core client");
    $core_app_version = new App_Version($core_app);
    $core_app_version->platform = $platform;
    $core_app_version->exec_dir = "../apps";
    $core_app_version->exec_name = "BOINC_0.12.exe";

    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->add_app($core_app);
    $project->add_app_version($core_app_version);
    $project->start_assimilator = false;
    $project->start_feeder = true;
    $project->start_file_deleter = false;
    $project->start_make_work = true;
    $project->start_result_retry = false;
    $project->start_validate = false;
    $project->shmem_key = 0x31415927;
    $project->project_php_file = "project_ap.inc";

    $project->install();

    $work = new Work($app);
    $work->wu_template = "pulse_wu";
    $work->result_template = "pulse_result";
    $work->redundancy = 5;
    array_push($work->input_files, "03au00ab_20575_00000.wu");
    $work->install($project);

    PassThru( "cd /mydisks/a/users/boincadm/html/apt/; ln -s ../download/ ./download");
    $project->start_feeder();
    $project->start_make_work($work);
?>
