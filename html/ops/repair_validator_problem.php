<?php

set_time_limit(0);
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

$result = mysql_query("select * from workunit where canonical_resultid=0");
while ($wu = mysql_fetch_object($result)) {
    $r2 = mysql_query("select count(*) from result where workunitid=$wu->id and outcome=1 limit 1000");
    $x = mysql_fetch_array($r2);
    mysql_free_result($r2);
    $nsuccess = $x[0];

    $r2 = mysql_query("select count(*) from result where workunitid=$wu->id and server_state=2");
    $x = mysql_fetch_array($r2);
    mysql_free_result($r2);
    $nunsent = $x[0];

    if ($nsuccess>=3 and $nunsent==0) {
        echo "WU $wu->id has $nsuccess success, $nunsent unsent \n";
        mysql_query("update workunit set need_validate=1 where id=$wu->id");
    }
}

?>
