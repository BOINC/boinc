<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("user.inc");
    require_once("host.inc");

    db_init();
    $hostid = $_GET["hostid"];
    $private = $_GET["private"];
    $ipprivate = $_GET["ipprivate"];
    $result = mysql_query("select * from host where id = $hostid");
    $host = mysql_fetch_object($result);
    mysql_free_result($result);
    if (!$host) {
        echo "Couldn't find computer";
        exit();
    }
    if ($private) {
        $user = get_logged_in_user();
        if (!$user || $user->id != $host->userid) {
            $private = false;
        }
    }

    page_head("Computer summary", $user);
    show_host($host, $private, $ipprivate);
    page_tail();
?>
