<?php

require_once("../inc/db.inc");

set_time_limit(0);

db_init();

function set_nusers() {
    $result = mysql_query("select id, nusers from team");
    while ($team = mysql_fetch_object($result)) {
        $r = mysql_query("select count(*) from user where teamid=$team->id");
        $x = mysql_fetch_array($r);
        mysql_free_result($r);
        $n = $x[0];
        if ($n != $team->nusers) {
            echo "team $team->id: $n $team->nusers\n";
            mysql_query("update team set nusers=$n where id=$team->id");
        }
    }
    mysql_free_result($result);
}

set_nusers();

?>
