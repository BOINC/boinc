<?php

require_once("util.inc");
require_once("team.inc");
require_once("db.inc");

db_init();
init_session();

$query = sprintf(
    "select * from team where id=%d",
    $HTTP_GET_VARS["id"]
);
$result = mysql_query($query);
if ($result) {
    $team = mysql_fetch_object($result);
}
if (!$team) {
    page_head("Unable to display team page");
    echo ("We are unable to display that team's page");
    page_tail();
} else {
    display_team_page($team);
}

?>
