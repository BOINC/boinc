#! /usr/local/bin/php
<?php
    // test the "dynamic result" feature
    //

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs1.xml");
    copy_to_download_dir("input");
    add_platform(null);
    //add_core_client();
    add_user("prefs.xml");
    add_app("upper_case", null, null);
    create_work("-appname upper_case -wu_name uc_wu -wu_template uc_wu -result_template uc_result -dynamic_results -nresults 2 input input");
    run_client("-exit_when_idle");

    check_results_done();
    compare_file("uc_wu_0_0", "uc_correct_output");
    compare_file("uc_wu_1_0", "uc_correct_output");
?>
