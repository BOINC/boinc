#! /usr/local/bin/php
<?php
    // test to make sure the scheduling server does not send back an unfeasible work unit using upper_case.
    //
    // You must have done "make" in all source directories

    include_once("init.inc");

    check_env_vars();
    clear_db();
    clear_data_dirs();
    create_keys();
    init_client_dirs("prefs3.xml");
    copy_to_download_dir("input");
    add_platform(null);
    add_user(null);
    add_app("upper_case", null, null);
    create_work("-appname upper_case -rsc_iops 180000000000.0 -rsc_fpops 0.0 -rsc_memory 1000000000000.2 -rsc_disk 1000000000000.2 -wu_name uc_wu -wu_template uc_wu -result_template uc_result -nresults 1 input");
    start_feeder();
    //run_client("-exit_after 10");
    run_client("-exit_when_idle > client_stdout");
    stop_feeder();

    PassThru("grep -c \"<scheduler_reply>\" client_stdout > tr_results", $retval);
    if($retval) printf("test_rsc did not run correctly\n");
    $f = fopen("tr_results", "r");
    fscanf($f, "%d", $value);
    if($value!=1) printf("test_rsc did not run correctly\n");
    else printf("test_rsc ran correctly\n");
    
    PassThru("rm -f tr_results client_stdout");
    
?>
