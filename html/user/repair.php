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
    for ($i=0; $i<825637; $i++) {
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

fix_validate_state();

?>
