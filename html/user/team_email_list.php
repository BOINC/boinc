<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

    db_init();

    $user = get_logged_in_user();
    $teamid = $_GET["teamid"];

    $result = mysql_query("select * from team where id=$teamid");
    if ($result) {
        $team = mysql_fetch_object($result);
        mysql_free_result($result);
    }

    require_founder_login($user, $team);

    page_head("$team->name Email List");
    start_table();
    row1("Member list of $team->name");
    row2_plain("<b>Name</b>", "<b>Email address</b>");
    $result = mysql_query("select * from user where teamid=$team->id");
    while ($user = mysql_fetch_object($result)) {
        if (!split_munged_email_addr($user->email_addr, null, $email)) {
            $email = $user->email_addr;
        }
        row2_plain($user->name, $email);
    } 
    mysql_free_result($result);
    end_table();

    page_tail();

?>
