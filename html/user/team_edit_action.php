<?php

require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/sanitize_html.inc");
require_once("../inc/boinc_db.inc");

$user = get_logged_in_user();
$teamid = post_int("teamid");
$team = BoincTeam::lookup_id($teamid);

if (!$team) error_page("no such team");
require_admin($user, $team);

$team_url = process_user_text(strip_tags(post_str("url", true)));
$x = strstr($team_url, "http://");
if ($x) {
    $team_url = substr($team_url, 7);
}
$team_name = process_user_text(strip_tags(post_str("name")));
$team_name_lc = strtolower($team_name);

$tnh = post_str("name_html", true);
if (get_magic_quotes_gpc()) $tnh = stripslashes($tnh);
$team_name_html = sanitize_html($tnh);

$team_name_html = process_user_text($team_name_html);

$team_description = process_user_text(post_str("description", true));
$type = process_user_text(post_str("type", true));
$country = process_user_text(post_str("country", true));
if ($country == "") {
    $country = "International";
}
if (!is_valid_country($country)) {
    error_page("bad country");
}

$t = BoincTeam::lookup("name='$team_name'");
if ($t && $t->id != $teamid) {
    error_page("The name '$team_name' is being used by another team.");
}
if (strlen($team_name) == 0) {
    error_page("Must specify team name");
    // Should be caught up with the post_str("name"),
    // but you can never be too safe.
}

$clause = sprintf(
    "name = '%s',
    name_lc = '%s',
    name_html = '%s',
    url = '%s',
    description = '%s',
    type = %d,
    country='%s'",
    $team_name,
    $team_name_lc,
    $team_name_html,
    $team_url,
    $team_description,
    $type,
    $country
);

$ret = $team->update($clause);
if ($ret) {
    Header("Location: team_display.php?teamid=$team->id");
} else {
    error_page("Could not update team - please try later.");
}

?>
