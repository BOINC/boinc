<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");
require_once("../inc/team.inc");

db_init();
$user = get_logged_in_user(true);
check_tokens($user->authenticator);

$teamid = get_int("teamid");
$team = lookup_team($teamid);
require_team($team);
if ($user->teamid == $team->id) {
    page_head("Unable to add $user->name");
    echo "You are already a member of $team->name.";
} else {
    $success = user_join_team($team, $user);
    if ($success) {
        Header("Location: home.php");
    } else {
        error_page("Couldn't join team - please try later.");
    }
}

page_tail();

?>
