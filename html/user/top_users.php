<?php
    require_once("db.inc");
    require_once("util.inc");
    require_once("user.inc");

    $authenticator = init_session();
    db_init();
    $numusers = 100;
    page_head("Top $numusers users");
    $result = mysql_query("select * from user order by expavg_credit desc,total_credit desc limit $numusers");
    user_table_start();
    while ($user = mysql_fetch_object($result)) {
        show_user_row($user);
    }
    echo "</table>\n";
    page_tail();
?>
