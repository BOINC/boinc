<?php
    require_once("../inc/db.inc");
    require_once("../inc/util.inc");

    init_session();
    db_init();
    page_head("Project totals");

    start_table();
    row1("Project totals");

    // need to break out the following by app

    if (0) {
        $result = mysql_query("select count(*) as total from result where server_state=5");
        $comp = mysql_fetch_object($result);
        mysql_free_result($result);
        row2("Results completed", $comp->total);

        $result = mysql_query("select count(*) as total from result where server_state=4");
        $comp = mysql_fetch_object($result);
        mysql_free_result($result);
        row2("Results in progress", $comp->total);

        $result = mysql_query("select count(*) as total from result where server_state=2");
        $comp = mysql_fetch_object($result);
        mysql_free_result($result);
        row2("Results waiting to send", $comp->total);
    }

    $result = mysql_query("select count(*) as total from user");
    $comp = mysql_fetch_object($result);
    mysql_free_result($result);
    row2("Participants", $comp->total);

    $result = mysql_query("select count(*) as total from host");
    $comp = mysql_fetch_object($result);
    mysql_free_result($result);
    row2("Computers", $comp->total);

    end_table();

    page_tail();

?>
