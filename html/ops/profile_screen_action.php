<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/db_ops.inc");
require_once("../inc/util_ops.inc");

$n = $_GET["n"];

db_init();

admin_page_head("screen profile action");
for ($i=0; $i<$n; $i++) {
    $y = "user".$i;
    $val = $_GET[$y];
    $x = "userid".$i;
    $userid = $_GET[$x];
    switch ($val) {
    case 1:
        mysql_query("update profile set verification=1 where userid=$userid");
        echo "<br>$userid is accepted";
        break;
    case -1:
        mysql_query("update profile set verification=-1 where userid=$userid");
        echo "<br>$userid is rejected";
        break;
    case 0:
        echo "<br>$userid is skipped";
        break;
    }
}

echo "
    <a href=\"profile_screen_form.php\">next 20</a>
";

admin_page_tail();

?>
