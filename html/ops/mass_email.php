<?php
require_once("../inc/db.inc");

db_init();

$result = mysql_query("select * from user where send_email>0");
while ($user = mysql_fetch_object($result)) {
    echo $user->email_addr;
}

?>
