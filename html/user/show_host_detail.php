<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("user.inc");
    require_once("host.inc");

    db_init();
    $hostid = $_GET["hostid"];
    $ipprivate = $_GET["ipprivate"];
    $host = lookup_host($hostid);
    if (!$host) {
        echo "Couldn't find computer";
        exit();
    }
    $private = false;
    $user = get_logged_in_user();
    if ($user && $user->id == $host->userid) {
        $private = true;
    }

    page_head("Computer summary", $user);
    show_host($host, $private, $ipprivate);
    page_tail();
?>
