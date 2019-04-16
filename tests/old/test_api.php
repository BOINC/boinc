#! /usr/local/bin/php
<?php
    include_once("init.inc");

    clean_api();
    run_api_test();
    compare_files("foobar", "ta_correct_f");
    clean_api();
?>

