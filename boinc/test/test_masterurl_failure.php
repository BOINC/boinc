#!/usr/local/bin/php -q
<?php {
    // $Id$

    $use_proxy_html = 1;

    include_once("test_uc.inc");
    test_msg("master url failure");

    $project = new ProjectUC;

    start_proxy('exit 1 if $nconnections < 4; if_done_kill(); if_done_ping();');

    // TODO: verify it took ??? seconds

    // TODO: time out after ??? seconds and fail this test

    $project->start_servers_and_host();
    $project->validate_all_and_stop();

    test_done();
} ?>
