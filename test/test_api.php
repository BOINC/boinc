#! /usr/local/bin/php
<?php
    include_once("init.inc");

    initialize_api();
    run_api_test();
    compare_files("app_to_core.xml", "ta_correct_atc");
    compare_files("foobar", "ta_correct_f");
?>

