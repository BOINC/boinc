<?php
    require_once("../inc/db.inc");

// activate/deactivate script
if (1) {
  echo "
This script needs to be activated before it can be run.
Once you understand what the script does you can change the 
if (1) to if (0) at the top of the file to activate it.
Be sure to deactivate the script after using it to make sure
it is not accidentally run. 
";
  exit;
}

    db_init();

function update_thread_timestamps() {
    $result = mysql_query("select * from thread");
    while ($thread = mysql_fetch_object($result)) {
    if (0) {
        $q = "select min(timestamp) as foo from post where thread=$thread->id";
        $r2 = mysql_query($q);
        $m = mysql_fetch_object($r2);
        echo "id: $thread->id; min: $m->foo\n";
        mysql_free_result($r2);
        $n = $m->foo;
        if ($n) {
            $q = "update thread set create_time=$n where id=$thread->id";
            mysql_query($q);
        }
    }
        $q = "select max(timestamp) as foo from post where thread=$thread->id";
        $r2 = mysql_query($q);
        $m = mysql_fetch_object($r2);
        echo "id: $thread->id; min: $m->foo\n";
        mysql_free_result($r2);
        $n = $m->foo;
        if ($n) {
            $q = "update thread set timestamp=$n where id=$thread->id";
            mysql_query($q);
        }
    }
    mysql_free_result($result);
}

function update_user_posts() {
    $result = mysql_query("select * from user");
    while ($user = mysql_fetch_object($result)) {
        $q = "select count(*) as num from post where user=$user->id";
        $r2 = mysql_query($q);
        $m = mysql_fetch_object($r2);
        mysql_free_result($r2);
        if ($m->num != $user->posts) {
            echo "user $user->id: $user->posts $m->num\n";
            $q = "update user set posts=$m->num where id=$user->id";
            mysql_query($q);
        }
    }
    mysql_free_result($result);
}

?>
