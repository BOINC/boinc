#!/usr/local/bin/php -q
<?php {
    // $Id$

    // tests makes sure that testing framework is sane

    include_once("test.inc");

    // make sure applications executable

    check_core_client_executable();
    check_app_executable("upper_case");
    check_app_executable("concat");
    check_app_executable("1sec");

    if (!is_dir(KEY_DIR)) {
        error("Keydir doesn't exist: ".KEY_DIR);
    }
    if (!is_dir(PROJECTS_DIR)) {
        error("Projects dir doesn't exist: ".PROJECTS_DIR);
    }
    if (!is_dir(HOSTS_DIR)) {
        error("Hosts dir doesn't exist: ".HOSTS_DIR);
    }
    if (!is_dir(CGI_DIR)) {
        error("CGI dir doesn't exist: ".CGI_DIR);
    }
    if (!is_dir(HTML_DIR)) {
        error("HTML dir doesn't exist: ".HTML_DIR);
    }
    if (!fopen(HTML_URL, 'r')) {
        error("Couldn't open html url: ".HTML_URL);
    }

    test_done();
} ?>
