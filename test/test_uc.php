#! /usr/local/bin/php
<?php
    // test the uc (upper-case) application
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    init_client_dirs("account1.xml");
    copy_to_download_dir("input");
    add_platform();
    add_user(null);
    add_app("upper_case");
    create_work("-appname upper_case -wu_name uc_wu -wu_template uc_wu -result_template uc_result -nresults 2 input input");
    run_client();

    check_results_done();
    compare_file("uc_wu_0_0", "uc_correct_output");
    compare_file("uc_wu_1_0", "uc_correct_output");
?>
