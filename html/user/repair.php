<?php
    require_once("db.inc");

    $retval = db_init();
    set_time_limit(10000);
    ob_end_flush();

    echo "init $retval\n";

// mark old unvalidated results as VALIDATE_STATE_NO_CHECK
//

function wu_over($wu) {
    if ($wu->transition_time==2147483647) return true;
    return false;
}

function fix_validate_state() {
    for ($i=458983; $i<825637; $i++) {
        $result = mysql_query("select * from workunit where id=$i");
        $wu = mysql_fetch_object($result);
        if ($wu) {
            if (wu_over($wu)) {
                echo "wu $wu->id\n";
                $r2 = mysql_query("select * from result where workunitid=$wu->id");
                while ($r = mysql_fetch_object($r2)) {
                    if ($r->validate_state == 0) {
                        echo " result $r->id $r->claimed_credit\n";
                        mysql_query("update result set validate_state=3 where id=$r->id");
                    }
                }
                mysql_free_result($r2);
            }
        }
        mysql_free_result($result);
    }
}

function host_credit($host) {
    $result = mysql_query("select sum(granted_credit) as total from result where hostid=$host->id");
    $foobar = mysql_fetch_object($result);
    mysql_free_result($result);
    mysql_query("update host set total_credit=$foobar->total where id=$host->id");
    echo "host $host->total_credit -> $foobar->total\n";
    return $foobar->total;
}

function assign_userid($result) {
    $host = lookup_host($result->hostid);
    if ($host) {
        mysql_query("update result set userid=$host->userid where id=$result->id");
    } else {
        mysql_query("update result set hostid=-1 where id=$result->id");
    }
}

function assign_userids_host($host) {
    $r = mysql_query("select * from result where hostid=$host->id");
    while ($result = mysql_fetch_object($r)) {
        if ($result->userid != $host->id) {
            mysql_query("update result set userid=$host->userid where id=$result->id");
        }
    }
    mysql_free_result($r);
}

function user_credit($userid) {
    $result = mysql_query("select * from host where userid=$userid");
    $x = 0;
    while ($host = mysql_fetch_object($result)) {
        echo "$host->id\n";
        assign_userids_host($host);
        $x += host_credit($host);
    }
    mysql_free_result($result);
    mysql_query("update user set total_credit=$x where id=$userid");
    echo "user $x\n";
}


function assign_userids() {
    while (1) {
        $r = mysql_query("select * from result where userid=0 and hostid>0 limit 1,100");
        $n = 0;
        while ($result = mysql_fetch_object($r)) {
            $n++;
            echo "$result->id\n";
            assign_userid($result);
        }
        mysql_free_result($r);
        if ($n==0) break;
    }
}

assign_userids();

//user_credit(132);

?>
