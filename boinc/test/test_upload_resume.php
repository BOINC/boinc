#!/usr/local/bin/php -q
<?php {
    // $Id$

    // This tests whether upload resuming works correctly.

    $use_proxy_cgi = 1;

    include_once("test_uc.inc");
    test_msg("upload resumes");

    $project = new ProjectUC;

    // TODO
    // start_proxy('exit 1 if ($nconnections ==1 && $bytes_transferred==ZZZ); if_done_kill(); if_done_ping();');

    $project->start_servers_and_host();
    $project->validate_all_and_stop();

    test_done();
} ?>
