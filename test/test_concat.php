#! /usr/local/bin/php
<?php
    // test the concat application
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    clear_db();
    clear_data_dirs();
    init_client_dirs("account1.xml");
    copy_to_download_dir("input");
    add_platform();
    add_core_client();
    add_user(null);
    add_app("concat");
    create_work("-appname concat -wu_name concat_wu -wu_template concat_wu -result_template concat_result -nresults 2 input input");
    run_client();
    compare_file("concat_wu_0_0", "concat_correct_output");
    compare_file("concat_wu_1_0", "concat_correct_output");
?>
