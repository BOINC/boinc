<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");

    db_init();

    $user = get_logged_in_user();

    $venue = $_GET["venue"];
    $hostid = $_GET["hostid"];

    $result = mysql_query("select * from host where id = $hostid");
    $host = mysql_fetch_object($result);
    mysql_free_result($result);

    if (!$host) {
        echo "Couldn't find host.";
        exit();
    }
    if ($host->userid != $user->id) {
        echo "Not your host\n";
        exit();
    }

    $retval = mysql_query("update host set venue='$venue' where id = $hostid");
    if ($retval) {
        page_head("Host venue updated");
        echo "
            The venue of this host has been set to <b>$venue</b>.
            <p>
            This change will take effect the next time
            the host requests work from the server,
            or when you Update this project from
            the BOINC Manager on the host.
            <p>
            <a href=show_host_detail.php?hostid=$hostid>Return to host page</a>.
        ";
        page_tail();
    } else {
        db_error_page();
    }
?>
