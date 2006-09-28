#! /usr/bin/php -q
<?php

// watchdog script to ensure that the number of result records
// increases at least every X seconds (X = crontab period)

include_once("db.inc");
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
    return $lines;
}

function write_count_file($n,$m) {
    $f = fopen("nresults", "w");
    if (!$f) return false;
    $x = sprintf("%d\n%d\n", $n, $m);
    fwrite($f, $x);
    fclose($f);
    return true;
}

function get_working_count_from_db() {
    $result = mysql_query("select count(*) from result where server_state = 4");
    if (!$result) return false;
    $count = mysql_fetch_array($result);
    mysql_free_result($result);
    return $count[0];
}

function get_count_from_db() {
    $result = mysql_query("select count(*) from result");
    if (!$result) return false;
    $count = mysql_fetch_array($result);
    mysql_free_result($result);
    return $count[0];
}

    $retval = db_init();
    if ($retval != 0) {
        fail("Can't open database\n");
    }
    $m = get_count_from_db();
    if ($m == false) {
        fail("Can't get result count from DB\n");
    }
    $p = get_working_count_from_db();
    if ($p == false) {
        fail("Can't get working result count from DB\n");
    }
    $n = read_count_file();
    if ($n == false) {
        write_count_file($m,$p);
        exit();
    }
    if (trim($n[0]) == $m && trim($n[1]) == $p) {
        fail("Result count hasn't changed\n");
    }
    write_count_file($m,$p);

?>
