<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

set_time_limit(0);

function set_nusers($credit_only) {
    echo "Running set_nusers() ...<br>\n";
    $result = mysql_query("select id, nusers from team");
    while ($team = mysql_fetch_object($result)) {
        $q = "select count(*) from user where teamid=$team->id";
        if ($credit_only) $q .= " and total_credit>0";
        echo $q . "<br>";
        $r = mysql_query($q);
        $x = mysql_fetch_array($r);
        mysql_free_result($r);
        $n = $x[0];
        if ($n != $team->nusers) {
            echo "team $team->id: old: $team->nusers new: $n<br>\n";
            mysql_query("update team set nusers=$n where id=$team->id");
        }
    }
    mysql_free_result($result);
    echo "set_nusers() finished!<br>";
}

db_init();
$use = $_GET['use'];

admin_page_head("Repair team nusers");

if ($use < 1 || $use >2) {
    echo "<br>Script to recalculate the number of users in a team.<br>
        Please choose:<br>
        <a href=\"team_repair.php?use=1\">Consider every user</a><br>
        <a href=\"team_repair.php?use=2\">Consider only user with total_credit>0</a><br>";
} else {
    if ($use == 1) {
        set_nusers(FALSE);
    } else {
        set_nusers(TRUE);
    }
}

admin_page_tail();
?>
