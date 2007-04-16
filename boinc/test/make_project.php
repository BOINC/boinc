#!/bin/sh

echo "this file is obsolete! cvs remove when ready"
exit 1


#! /usr/local/bin/php
<?php
    // This script creates a BOINC project.
    // You just need to plug in an application,
    // and back-end systems for creating work and validating results

    include_once("test.inc");

    $project = new Project;
    $project->short_name = "project_name";
    $project->long_name = "Your Project";

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
    $core_app_version->exec_name = "BOINC_0.10.exe";

    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->add_app($core_app);
    $project->add_app_version($core_app_version);
    $project->start_assimilator = true;
    $project->start_feeder = true;
    $project->start_file_deleter = true;
    $project->start_make_work = true;
    $project->start_timeout_check = true;
    $project->start_validate = true;
    $project->shmem_key = 0x31415926;

    $project->install();
    $project->start_feeder();
?>
