<?php
require_once("../inc/cache.inc");

$sort_by = $_GET["sort_by"];
if (!$sort_by) $sort_by = "expavg_credit";
$offset = $_GET["offset"];
if (!$offset) $offset=0;
$teamid = $_GET["teamid"];

if ($offset > 1000) {
    page_head("Limit exceeded");
    echo "Sorry, we're currently showing only the first 1000.";
    page_tail();
    exit();
}

$cache_args = "teamid=$teamid&sort_by=$sort_by&offset=$offset";
start_cache(TEAM_PAGE_TTL, $cache_args);

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(false);

$result = mysql_query("select * from team where id=$teamid");
if ($result) {
    $team = mysql_fetch_object($result);
}
if (!$team) {
    echo ("Can't find team in database");
    exit();
}

display_team_page($team, $offset, $sort_by);

end_cache(TEAM_PAGE_TTL,$cache_args);

?>
