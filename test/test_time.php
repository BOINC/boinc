#! /usr/local/bin/php
<?php
    // test the time reporting mechanism in the client
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    $app_time = 0;
    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs1.xml");
    copy_to_download_dir("small_input");
    add_platform(null);
    add_core_client(null);
    add_user("prefs.xml");
    add_app("uc_cpu", null, null);
    create_work("-appname uc_cpu -rsc_fpops 4500000000.0 -rsc_iops 500000000.0 -wu_name uccpu_wu -wu_template uccpu_wu -result_template uccpu_result -nresults 1 small_input");
    start_feeder();
    run_client("-exit_after 400");
    $app_time += get_time("app.time");
    PassThru("cp client_state.xml client_state.xml.one");
    run_client("-exit_when_idle");
    $app_time += get_time("app.time");
    compare_file("uccpu_wu_0_0", "uc_small_correct_output");
    compare_time($app_time);
?>
