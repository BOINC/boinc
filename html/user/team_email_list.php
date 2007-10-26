<?php
$cvs_version_tracker[]="\$Id$";  //Generated automatically - do not edit

require_once("../inc/boinc_db.inc");
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
    if (!$user || $team->userid != $user->id || $teamid != $user->teamid) {
        $show_email = false;
    }
    echo "<users>\n";
    $users = BoincUser::enum("teamid=$team->id");
    foreach($users as $user) {
        show_team_member($user, $show_email);
    } 
    echo "</users>\n";
    exit();
}

$user = get_logged_in_user();
$teamid = get_int("teamid");
$plain = get_int("plain", true);
$team = BoincTeam::lookup_id($teamid);
if (!$team) {
    error_page("no such team");
}
require_founder_login($user, $team);

if ($plain) {
    header("Content-type: text/plain");
} else {
    page_head("$team->name Email List");
    start_table();
    table_header(array("Member list of ".$team->name, "colspan=\"5\""));
    table_header("Name", "Email address", "Total credit", "Recent average credit", "Country");
}
$users = BoincUser::enum("teamid=$team->id");
foreach($users as $user) {
    if ($plain) {
        echo "$user->name <$user->email_addr>\n";
    } else {
        table_row(user_links($user), $user->email_addr, format_credit($user->total_credit), format_credit($user->expavg_credit), $user->country);
    }
} 
if (!$plain) {
    end_table();
    echo "<p><a href=\"team_email_list.php?teamid=".$teamid."&amp;plain=1\">Show as plain text</a></p>";
    page_tail();
}

?>
