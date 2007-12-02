<?php

require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/cache.inc");
require_once("../inc/cache.inc");

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;

if ($offset > 1000) {
    error_page("Limit exceeded:  Can only display the first 1000 members.");
}

$teamid = get_int("teamid");

$cache_args = "teamid=$teamid&offset=$offset&sort_by=$sort_by";
start_cache(TEAM_PAGE_TTL, $cache_args);

$team = BoincTeam::lookup_id($teamid);

page_head("Members of <a href=team_display.php?teamid=$teamid>$team->name</a>");
display_team_members($team, $offset, $sort_by);
page_tail();

end_cache(TEAM_PAGE_TTL, $cache_args);

?>
