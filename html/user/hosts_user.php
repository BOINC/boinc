<?php
    // show all the hosts for a user.
    // if $userid is absent, show hosts of logged-in user

    require_once("util.inc");
    require_once("host.inc");

    db_init();
    $userid = $_GET["userid"];
    if ($userid) {
        $result = mysql_query("select * from user where id=$userid");
        $user = mysql_fetch_object($result);
        mysql_free_result($result);
        page_head("Computers belonging to $user->name");
        host_table_start("Computers belonging to $user->name", false);
        $private = false;
    } else {
        $user = get_logged_in_user();
        $userid = $user->id;
        page_head("Your computers");
        host_table_start("Your computers", true);
        $private = true;
    }
    $i = 1;
    $result = mysql_query("select * from host where userid=$userid order by expavg_credit desc");
    while ($host = mysql_fetch_object($result)) {
        show_host_row($host, $i, $private);
        $i++;
    }
    mysql_free_result($result);
    echo "</table>\n";
    page_tail();
?>
