<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");

db_init();

admin_page_head("Manage special users action");

$bitset = '';

for ($i=0;$i<=6;$i++) {
    if (post_int($i, TRUE) == '1') {
        $bitset = str_pad($bitset, $i+1, '1');
    } else {
        $bitset = str_pad($bitset, $i+1, '0');
    }
}
if ($bitset == "0000000") $bitset = '';
$userid = post_int("userid");

$query = "UPDATE forum_preferences SET special_user='$bitset' WHERE userid='$userid'";
mysql_query($query);

if (mysql_affected_rows() == 1) {
    echo "<center><h2>Success</h2>";
} else {
    echo "<center><h2>Failure</h2>";
}

echo "Query was: $query</center>";

//echo "<br><a href=\"manage_special_users.php\">Manage users</a>";

admin_page_tail();

?>
