#! /usr/local/bin/php
<?php
    // test the uc (upper-case) application
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs1.xml");
    copy_to_download_dir("input");
    add_platform(null);
    add_user("prefs.xml");
    add_app("upper_case", null, null);
    create_work("-appname upper_case -rsc_iops 180000000000.0 -rsc_fpops 0.0 -wu_name uc_wu -wu_template uc_wu -result_template uc_result -nresults 2 input input input input input");
    start_feeder();
    //run_client("-exit_after 10");
    run_client("-exit_when_idle");
    stop_feeder();
    check_results_done();
    compare_file("uc_wu_0_0", "uc_correct_output");
    compare_file("uc_wu_1_0", "uc_correct_output");
?>
