<?php

require_once("../inc/db.inc");

db_init();
$hostid = $_GET["hostid"];

if (!$hostid) {
    echo "no host ID\n";
    exit();
}

mysql_query("update host set rpc_time=0 where id=$hostid");
echo "Host RPC time cleared\n";

?>
