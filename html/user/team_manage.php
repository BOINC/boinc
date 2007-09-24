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
        <br><span class=note>Change team name, URL, description, type, or country</span>
    <li><a href=team_remove_inactive_form.php?teamid=$team->id>Remove members</a>
        <br><span class=note>Remove inactive or unwanted members from this team</span>
    <li>
        Member list:
    <a href=team_email_list.php?teamid=$team->id>HTML</a>
    | <a href=team_email_list.php?teamid=$team->id>text</a>
        <br><span class=note> View member names and email addresses </span>
    <li><a href=team_change_founder_form.php?teamid=$team->id>Change founder</a>
        <br><span class=note>Transfer foundership to another member</span>
    <li>View change history:
    <a href=team_delta.php?teamid=$team->id>HTML</a>
        | <a href=team_delta.php?teamid=$team->id&xml=1>XML</a>
        <br><span class=note>See when members joined or quit this team</span>

    <li>
        To have this team created on all BOINC projects
        (current and future) you can make it into a
        <a href=http://boinc.berkeley.edu/teams/>BOINC-wide team</a>.
    <li>
        Other resources for BOINC team founders
        are available from a third-party site,
        <a href=http://www.boincteams.com>www.boincteams.com</a>.
    </ul>
<hr>
";

page_tail();
?>
