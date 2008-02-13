<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/team.inc");

page_head("Teams");

echo "<p>".PROJECT." participants may form <b>teams</b>.
    <p>
    You may belong to only one team.
    You can join or quit a team at any time.
    <p>
    Each team has a <b>founder</b>, who may
    <ul>
    <li> access team members' email addresses
    <li> edit the team's name and description
    <li> remove members from the team
    <li> disband a team if it has no members
    </ul>
    <p>
    To join a team, visit its team page and click <b>Join this team</b>.
    <h3>Find a team</h3>
";
team_search_form(null);
echo "

    <h3>Top teams</h3>
    <ul>
    <li> <a href=top_teams.php>All teams</a>
";

for ($i=1; $i<8; $i++) {
    echo "<li> <a href=top_teams.php?type=$i>".team_type_name($i)." teams</a>
    ";
}

echo "
    </ul>
    <h3>Create a new team</h3>
    If you can't find a team that's right for you, you can
    <a href=team_create_form.php>create a team</a>.
";
page_tail();

?>
