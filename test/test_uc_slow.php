#! /usr/local/bin/php
<?php
    // test the client checkpoint/restart mechanism,
    // using the uc_slow application
    //

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs1.xml");
    copy_to_download_dir("small_input");
    add_platform(null);
    add_core_client(null);
    add_user("prefs.xml");
    add_app("uc_slow", null, null);
    create_work("-appname uc_slow -wu_name ucs_wu -wu_template ucs_wu -result_template ucs_result -nresults 1 small_input");
    echo "Now run the client manually; start and stop it a few times.\n";
    start_feeder();
    //run_client();
    //compare_file("ucs_wu_0_0", "uc_small_correct_output");
?>
