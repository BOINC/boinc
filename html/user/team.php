<?php

include_once("../inc/db.inc");
include_once("../inc/util.inc");
include_once("../inc/team.inc");

init_session();
db_init();

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
    To join a team, visit its team page and click Join.
    <p>
    Each team has a <a href=clone.php>Create team account URL</a>;
    accounts created through this URL will belong to the team,
    and will have the project preferences of its founder.
    <hr>
    <ul>
    <li> <form method=get action=team_lookup.php>
    Search for teams whose name contains with:
    <input name=team_name>
    <input type=submit name=search value=Search>
    </form>
    <li> <a href=team_create_form.php>Create a team</a>
    </ul>
    Show top teams:
    <ul>
    <li> <a href=top_teams.php>All teams</a>
";

for ($i=1; $i<8; $i++) {
    echo "<li> <a href=top_teams.php?type=$i>".team_type_name($i)." teams</a>
    ";
}

echo "
    </ul>
";
    page_tail();

?>
