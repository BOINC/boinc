<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/email.inc");
require_once("../inc/team.inc");

$xml = get_int('xml', true);
if ($xml) {
    require_once("../inc/xml.inc");
    xml_header();
    $retval = db_init_xml();
    if ($retval) xml_error($retval);
    $teamid = get_int("teamid");
    $team = lookup_team($teamid);
    if (!$team) {
        xml_error(-136);
    }
    $show_email = true;
    $account_key = get_str('account_key', true);
    $user = lookup_user_auth($account_key);
    if (!$user || $team->userid != $user->id) {
        $show_email = false;
    }
    echo "<users>
";
    $result = mysql_query("select * from user where teamid=$team->id");
    while ($user = mysql_fetch_object($result)) {
        show_team_member($user, $show_email);
    } 
    echo "</users>
";
    exit();
}

db_init();

$teamid = get_int("teamid");
$team = lookup_team($teamid);
$user = get_logged_in_user();
require_founder_login($user, $team);

page_head("$team->name Email List");
start_table();
row1("Member list of $team->name");
row2_plain("<b>Name</b>", "<b>Email address</b>");
$result = mysql_query("select * from user where teamid=$team->id");
while ($user = mysql_fetch_object($result)) {
    row2_plain($user->name, $user->email_addr);
} 
mysql_free_result($result);
end_table();

page_tail();

?>
