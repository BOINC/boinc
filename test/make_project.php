#! /usr/local/bin/php
<?php
    // This script creates a BOINC project.
    // You just need to plug in an application,
    // and back-end systems for creating work and validating results

    include_once("test.inc");

    $project = new Project;
    $app = new App("your_app_name_here");
    $app_version = new App_Version($app);

    $project->add_app($app);
    $project->add_app_version($app_version);
    $project->install();
    $project->start();
?>
