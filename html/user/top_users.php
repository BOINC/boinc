<?php
    require_once("util.inc");
    require_once("user.inc");

    $authenticator = init_session();
    db_init();
    page_head("Top users");
    $result = mysql_query("select * from user order by expavg_credit desc,total_credit desc");
    user_table_start();
    while ($user = mysql_fetch_object($result)) {
        show_user_row($user);
    }
    echo "</table>\n";
    page_tail();
?>
