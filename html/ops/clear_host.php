<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

db_init();
$hostid = $_GET["hostid"];

if (!$hostid) {
    echo "no host ID\n";
    exit();
}

mysql_query("update host set rpc_time=0 where id='$hostid'");
echo "Host RPC time cleared for hostid: $hostid\n";

admin_page_tail();
?>
