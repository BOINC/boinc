#! /usr/local/bin/php
<?php
    // test the seti@home client
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    check_env_vars();  // THIS IS ABSOLUTELY CRITICAL!!!
    clear_db();
    clear_data_dirs();
    init_client_dirs("account_sah.xml");
    copy_to_download_dir("work_unit.sah");
    add_platform();
    add_core_client();
    add_user(null);
    add_app("setiathome-3.06");
    create_work("-appname setiathome-3.06 -wu_name sah_wu -wu_template sah_wu -result_template sah_result -nresults 1 work_unit.sah");
    echo "starting feeder\n";
    start_feeder();
    echo "started feeder\n";
    run_client();
    echo "ran client\n";
    stop_feeder();

    check_results_done();
?>
