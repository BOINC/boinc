<?php

// show a result

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/result.inc");

db_init();
$resultid = get_int("resultid");
$result = lookup_result($resultid);
if (!$result) {
    error_page("No such result");
}
page_head("Result");
show_result($result);
page_tail();

?>
