#! /usr/local/bin/php
<?php
    // test multiple projects
    //

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs2.xml");
    copy_to_download_dir("small_input");
    add_platform();
    add_core_client();
    add_user(null);
    add_app("upper_case");
    create_work("-appname upper_case -wu_name uc_wu -wu_template uc_wu -result_template uc_result -nresults 2 small_input");
    start_feeder();
    run_client();
    stop_feeder();
?>
