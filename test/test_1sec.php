#! /usr/local/bin/php
<?php
    // test the 1sec application

    include_once("init.inc");

    clear_db();
    clear_data_dirs();
    init_client_dirs("account2.xml");
    add_platform();
    add_core_client();
    add_user();
    add_app("1sec");
    create_work("-appname 1sec -wu_name 1sec_wu -wu_template 1sec_wu -result_template 1sec_result -nresults 10");
    //run_client();
    //compare_file("concat_wu_0_0", "1sec_correct_output");
    //compare_file("concat_wu_1_0", "1sec_correct_output");
?>
