<?php
    require_once("util.inc");
    require_once("db.inc");

    db_init();

    echo "<hr>Platforms";
    $result = mysql_query("select * from platform");
    while ($platform = mysql_fetch_object($result)) {
        show_platform($platform);
    }

    echo "<hr>Apps";
    $result = mysql_query("select * from app");
    while ($app = mysql_fetch_object($result)) {
        show_app($app);
    }
    echo "<hr>App versions";
    $result = mysql_query("select * from app_version");
    while ($app_version = mysql_fetch_object($result)) {
        show_app_version($app_version);
    }
    echo "<hr>Hosts";
    $result = mysql_query("select * from host");
    while ($host = mysql_fetch_object($result)) {
        show_host($host);
    }
    echo "<hr>Workunits";
    $result = mysql_query("select * from workunit");
    while ($workunit = mysql_fetch_object($result)) {
        show_workunit($workunit);
    }
    echo "<hr>Results";
    $result = mysql_query("select * from result");
    while ($res = mysql_fetch_object($result)) {
        show_result($res);
    }
    echo "<hr>Users";
    $result = mysql_query("select * from user");
    while ($user = mysql_fetch_object($result)) {
        show_user($user);
    }
?>
