<?php

// cancel a WU:
// - mark unsent results as OVER, outcome DIDNT_NEED
// - set CANCELLED bit in WU error mask
//
function cancel_wu($wuid) {
    mysql_query("update result set server_state=5, outcome=5 where server_state=2 and workunitid=$wuid);

    mysql_query("update workunit set error_mask=error_mask|16 where id=$wuid");
}


require_once("../inc/db.inc");

// REMOVE THE FOLLOWING ONLY AFTER PASSWORD-PROTECTING html/ops
//
if (1) {
    echo "
        Make sure the html/ops directory is password-protected,
        then edit cancel_wu.php to remove this message.
    ";
    exit();
}

db_init();

$wuid = $_GET["wuid"];

cancel_wu($wuid);

echo "WU $wuid has been cancelled.";

?>
