<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");

    db_init();

    $user = get_logged_in_user();

    $venue = get_venue("venue");
    $hostid = get_int("hostid");

    $host = lookup_host($hostid);
    if (!$host) {
        error_page("No such host");
    }
    if ($host->userid != $user->id) {
        error_page("Not your host");
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
