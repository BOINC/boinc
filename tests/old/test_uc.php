#!/usr/local/bin/php -q
<?php {
    // $Id$

    // This tests whether the most basic mechanisms are working
    // Also whether stderr output is reported correctly
    // Also tests if water levels are working correctly

    include_once("test_uc.inc");
    test_msg("standard upper_case application");

    $project = new ProjectUC;
    $project->start_servers_and_host();
    $project->validate_all_and_stop();

    test_done();
} ?>
