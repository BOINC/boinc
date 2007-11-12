<?php

require_once("../inc/boinc_db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

$user = get_logged_in_user();
$teamid = post_int("id");
$team = BoincTeam::lookup_id($teamid);
if (!$team) error_page("No such team");
require_founder_login($user, $team);

page_head("Removing users from $team->name");
$ndel = 0;
for ($i=0; $i<$_POST["ninactive_users"]; $i++) {
    $userid = post_int("remove_$i", true);
    if (!$userid) continue;
    $user = BoincUser::lookup_id($userid);
    if (!$user) continue;
    if ($user->teamid != $team->id) {
        echo "<br>$user->name is not a member of $team->name";
    } else {
        user_quit_team($user);
        echo "<br>$user->name has been removed";
        $ndel++;
    }
}

page_tail();

?>
