<?php

require_once("../inc/db.inc");
require_once("../inc/xml.inc");
require_once("../inc/team.inc");

db_init();

xml_header();

function reply($x) {
    echo "<create_team_reply>
    $x
</create_team_reply>
";
    exit();
}

function error($x) {
    reply("<error>$x</error>");
}

function success($x) {
    reply("<success/>\n$x");
}

$auth = process_user_text($_GET["account_key"]);
$user = lookup_user_auth($auth);
if (!$user) {
    error("no such user");
}

$name = process_user_text(strip_tags($_GET["name"]));
if (strlen($name) == 0) {
    error("must set team name");
}
$name_lc = strtolower($name);

$url = process_user_text(strip_tags($_GET["url"]));
if (strstr($url, "http://")) {
    $url = substr($url, 7);
}

$type = process_user_text(strip_tags($_GET["type"]));
if (strlen($type) == 0) {
    $type = 1;
}
$name_html = process_user_text($_GET["name_html"]);
$description = process_user_text($_GET["description"]);
$country = process_user_text($_GET["country"]);
if (!is_valid_country($country)) {
    $country = 'None';
}

$query = sprintf(
    "insert into team (userid, create_time, name, name_lc, url, type, name_html, description, country, nusers, expavg_time) values (%d, %d, '%s', '%s', '%s', %d, '%s', '%s', '%s', %d, unix_timestamp())",
    $user->id, time(), $name, $name_lc, $url, $type, $name_html, $description, $country, 0
);
$result = mysql_query($query);

if ($result) {
    $teamid = mysql_insert_id();
    $team_result = mysql_query("select * from team where id = $teamid");
    $new_team = mysql_fetch_object($team_result);
    mysql_free_result($team_result);
    user_join_team($new_team, $user);
    success("<team_id>$teamid</team_id>");
} else {
    error("could not create team");
}

?>
