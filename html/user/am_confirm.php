<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

$nonce = process_user_text($_GET["nonce"]);

$query = "select * from tentative_user where nonce='$nonce'";
$result = mysql_query($query);
$tuser = mysql_fetch_object($result);
mysql_free_result($result);

if ($tuser) {
    $result = mysql_query("update tentative_user set confirmed=1 where nonce='$nonce'");
    if ($result) {
        page_head("Account confirmed");
        page_tail();
    } else {
        echo "Can't update database - please retry later";
    }
} else {
    echo "No such user";
}
