#! /usr/local/bin/php
<?php
    // test the time reporting mechanism in the client
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    check_env_vars();
    clear_db();
    create_keys();
    init_client_dirs("prefs1.xml");
    copy_to_download_dir("small_input");
    add_platform(null);
    add_core_client(null);
    add_user("prefs.xml");
    add_app("uc_slow", null, null);
    create_work("-appname uc_slow -wu_name ucs_wu -wu_template ucs_wu -result_template ucs_result -nresults 1 small_input");
    start_feeder();
    run_client("-exit_when_idle");
    compare_file("ucs_wu_0_0", "uc_small_correct_output");
    compare_time();
?>
