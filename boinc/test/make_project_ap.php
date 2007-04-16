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
    $project->short_name = "apt";
    $project->long_name = "Astropulse";

    // Solaris version
    $platform_sol = new Platform("sparc-sun-solaris2.7", "Solaris");

    $app = new App("Astropulse");
    $app_version_sol = new App_Version($app);
    $app_version_sol->platform = $platform_sol;
    $app_version_sol->exec_dir = "../apps";
    $app_version_sol->version = 9;
    $app_version_sol->exec_name = "ap_sparc_0.09";

    // Linux version
    $platform_lin = new Platform("i686-pc-linux-gnu", "Linux");

    $app_version_lin = new App_Version($app);
    $app_version_lin->platform = $platform_lin;
    $app_version_lin->exec_dir = "../apps";
    $app_version_lin->version = 9;
    $app_version_lin->exec_name = "ap_linux_0.09";

    /*
    $core_app = new App("core client");
    $core_app_version = new App_Version($core_app);
    $core_app_version->platform = $platform;
    $core_app_version->exec_dir = "../apps";
    $core_app_version->version = 13;
    $core_app_version->exec_name = "BOINC_0.13a.exe";
    */

    $project->add_app($app);
    $project->add_app_version($app_version_sol);
    $project->add_app_version($app_version_lin);
    //$project->add_app($core_app);
    //$project->add_app_version($core_app_version);
    $project->shmem_key = 0x3141a666;
    $project->project_php_file = "project_ap.inc";
    $project->project_prefs_php_file = "project_specific_prefs_ap.inc";

    $project->install();
    $project->install_feeder();
    $project->install_assimilator($app);
    $project->install_file_delete();
    $project->install_validate($app,3);
    $project->start_servers();

    /*$work = new Work($app);
    $work->wu_template = "pulse_wu";
    $work->result_template = "pulse_result";
    $work->redundancy = 5;
    array_push($work->input_files, "03au00ab_20575_00000.wu");
    $work->install($project);*/
?>
