<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db.inc");
require_once("../inc/util_ops.inc");

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
$hostid = $_GET["hostid"];

if (!$hostid) {
    echo "no host ID\n";
    exit();
}

mysql_query("update host set rpc_time=0 where id='$hostid'");
echo "Host RPC time cleared for hostid: $hostid\n";

admin_page_tail();
?>
