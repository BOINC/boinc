#! /usr/local/bin/php
<?php
    // test the 1sec application
    // This tests whether CPU time is divided correctly between projects
    // TODO: make this test what it's supposed to test

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    init_client_dirs("account2.xml");
    add_platform();
    add_core_client();
    add_user(null);
    add_app("1sec");
    create_work("-appname 1sec -wu_name 1sec_wu -wu_template 1sec_wu -result_template 1sec_result -nresults 10");
    start_feeder();
    run_client();
    stop_feeder();
?>
