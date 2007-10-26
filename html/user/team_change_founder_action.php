<?php

require_once("../inc/util.inc");
require_once("../inc/team.inc");
require_once("../inc/boinc_db.inc");

$user = get_logged_in_user();

$teamid = post_int("teamid");
$team = BoincTeam::lookup_id($teamid);
if (!$team) {
    error_page("No such team");
}
require_founder_login($user, $team);

page_head("Changing founder of $team->name");
$n = $_POST["navailable_users"];
for ($i=0; $i<$n; $i++) {
    if ($_POST["change_$i"] != 0) {
        $userid = post_int("change_$i");
        $user = BoincUser::lookup_id($userid);
        if ($user->teamid != $team->id) {
            echo "<br>$user->name is not a member of $team->name";
        } else {
            $team->update("userid=$userid, ping_user=0");
            echo "<br>$user->name is now founder of $team->name";
        }
    }
}
page_tail();

?>
