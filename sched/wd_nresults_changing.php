#! /usr/local/bin/php
<?php

// watchdog script to ensure that the number of result records
// increases at least every X seconds (X = crontab period)

include_once("util.inc");

function fail($x) {
    $f = fopen("error_log", "a");
    if (!$f) return false;
    fputs($f, "[".strftime("%T %D")."] ");
    fputs($f, $x);
    fclose($f);
    exit();
}

function read_count_file() {
    if (!file_exists("nresults")) {
        return false;
    }
    $lines = file("nresults");
    if (!$lines) return false;
    return $lines[0];
}

function write_count_file($n) {
    $f = fopen("nresults", "w");
    if (!$f) return false;
    $x = sprintf("%d", $n);
    fwrite($f, $x);
    fclose($f);
    return true;
}

function get_count_from_db() {
    $result = mysql_query("select count(*) from result");
    if (!$result) return false;
    $count = mysql_fetch_array($result);
    mysql_free_result($result);
    return $count[0];
}

    $retval = db_init();
    if (!$retval) {
        fail("Can't open database");
    }
    $m = get_count_from_db();
    echo "Count: $m\n";
    if ($m == false) {
        fail("Can't get result count from DB");
    }
    $n = read_count_file();
    echo "Count: $n\n";
    if ($n == false) {
        write_count_file($m);
        exit();
    }
    if ($n == $m) {
        //echo "fail\n";
        fail("Result count hasn't changed\n");
    }
    write_count_file($m);

?>
