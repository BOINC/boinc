#! /usr/local/bin/php
<?php
    // test preferences
    //

    include_once("init.inc");

    clear_db();
    clear_data_dirs();
    init_client_dirs("account1.xml");
    copy_to_download_dir("small_input");
    add_platform();
    add_core_client();
    add_user("min_water_prefs.xml");
    add_app("uc_slow");
    create_work("-appname uc_slow -wu_name ucs_wu -wu_template ucs_wu -result_template ucs_result -nresults 1 small_input");
    //will automatically run client instead
    //echo "Now run the client manually; start and stop it a few times.\n";
    //run_client();
    //compare_file("ucs_wu_0_0", "uc_small_correct_output");
    echo "starting feeder\n";
    start_feeder();
    echo "started feeder\n";
    run_client();
    echo "ran client\n";
    stop_feeder();
    //not sure what correct results should be, so will have to figure 
    //this out so can run diff
?>
