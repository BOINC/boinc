<?php
require_once("../inc/cache.inc");
require_once("../inc/util.inc");

$sort_by = $_GET["sort_by"];
if (!$sort_by) $sort_by = "expavg_credit";
$offset = get_int("offset", true);
if (!$offset) $offset=0;
$teamid = get_int("teamid");

if ($offset > 1000) {
    page_head("Limit exceeded");
    echo "Sorry, we're currently showing only the first 1000.";
    page_tail();
    exit();
}

$cache_args = "teamid=$teamid&sort_by=$sort_by&offset=$offset";
start_cache(TEAM_PAGE_TTL, $cache_args);

require_once("../inc/db.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(false);

$team = lookup_team($teamid);
if (!$team) {
    error_page("No such team");
}

display_team_page($team, $offset, $sort_by);

page_tail(true);
end_cache(TEAM_PAGE_TTL,$cache_args);

?>
