<?php

require_once("../inc/db.inc");
require_once("../inc/util.inc");

db_init();

$user = get_logged_in_user(true);

page_head("Team management");
$team = lookup_team($user->teamid);

if (!$team) {
    echo "no team";
    exit();
}
if ($team->userid != $user->id) {
    echo "Not founder";
    exit();
}

echo "
    <ul>
    <li><a href=team_edit_form.php?teamid=$team->id>Edit team info</a>
    <li><a href=team_remove_inactive_form.php?teamid=$team->id>Remove inactive members</a>
    <li><a href=team_email_list.php?teamid=$team->id>View team email addresses</a>
    <li><a href=team_change_founder_form.php?teamid=$team->id>Change founder</a>
    <li><a href=team_delta.php?teamid=$team->id>View change history</a>
        <a href=team_delta.php?teamid=$team->id&xml=1>(XML)</a>
    </ul>
";

page_tail();
?>
