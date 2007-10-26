<?php
require_once("../inc/cache.inc");
require_once("../inc/util.inc");
require_once("../inc/db.inc");
require_once("../inc/team.inc");

if (isset($_GET["sort_by"])) {
    $sort_by = $_GET["sort_by"];
} else {
    $sort_by = "expavg_credit";
}

$offset = get_int("offset", true);
if (!$offset) $offset=0;
$teamid = get_int("teamid");

if ($offset > 1000) {
    error_page("Limit exceeded:  Only displaying the first 1000 members.");
}

db_init();

$get_from_db = false;

$user = get_logged_in_user(false);

// always show fresh copy to founder; they might be editing info
//
if ($user && $user->teamid == $teamid) {
    $get_from_db = true;
}

// We can only cache team object, as the page is customised to the current user

$cache_args = "teamid=$teamid&sort_by=$sort_by&offset=$offset";
if (!$get_from_db) {
    $cached_data = get_cached_data(TEAM_PAGE_TTL, $cache_args);
    if ($cached_data) {
        // We found some old but non-stale data, let's use it
        $team = unserialize($cached_data);
    } else {
        $get_from_db = true;
    }
}
if ($get_from_db) {
    $team = BoincTeam::lookup_id($teamid);
    $team->nusers = BoincUser::count("teamid=$teamid and total_credit>0");
    set_cache_data(serialize($team), $cache_args);
}

if (!$team) {
    error_page("No such team");
}

display_team_page($team, $offset, $sort_by);

page_tail(true);

?>
